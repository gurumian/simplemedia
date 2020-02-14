#include "media_player.h"
#include <iostream>
#include <fcntl.h>
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <map>
#include <functional>
#include <stdlib.h>

#include "log_message.h"

#include <simplemedia/sdl/sdl_audio_renderer.h>
#include <simplemedia/sdl/sdl_video_renderer.h>
#include <simplemedia/sdl/sdl_subtitle_renderer.h>

#include <future>

using namespace std::placeholders;

namespace gurum {

MediaPlayer::MediaPlayer() {
  sync_threshold_ = kDefaultSyncThreshold;
}

MediaPlayer::~MediaPlayer() {
  if(log_enabled_) LOG(INFO) << __func__;
}

int MediaPlayer::Prepare(OnPrepared on_prepared) {
  std::unique_lock<std::mutex> lk(lck_);

  std::promise<void> promise;
  std::future<void> fut = promise.get_future();

  using namespace std::placeholders;
  source_->EnableLog(log_enabled_);
  source_->PrepareAsync([&](const AVFormatContext *fmt)->int {
    if(source_->HasVideo()) {
      int pid = source_->videoPid();
      PidChannel *pidchannel = source_->RequestPidChannel(pid);
      if(! pidchannel) {
        LOG(ERROR) << " failed to request a pid-channel for video";
        return -1;
      }

      video_decoder_.reset(new VideoDecoder);
      video_decoder_->Prepare(fmt->streams[pid]);
      video_decoder_->SetPidChannel(pidchannel);

      video_decoder_->SetOnFrameFound(
          std::bind(&MediaPlayer::OnVideoFrameFound, this, _1)
      );

      video_decoder_->SetOnNullPacketSent(
          std::bind(&MediaPlayer::OnNullPacketSent, this, _1)
      );

      if(video_renderer_) {
        int err = video_renderer_->Prepare(
            video_decoder_->width(),
            video_decoder_->height(),
            video_decoder_->pixelFormat());
        if(err) {
          LOG(ERROR) << " failed to prepare a video renderer.";
          return err;
        }
      }
    }

    if(source_->HasAudio()) {
      int pid = source_->audioPid();
      PidChannel *pidchannel = source_->RequestPidChannel(pid);
      if(! pidchannel) {
        LOG(ERROR) << " failed to request a pid-channel for audio";
        return -1;
      }

      audio_decoder_.reset(new AudioDecoder);
      audio_decoder_->Prepare(fmt->streams[pid]);
      audio_decoder_->SetPidChannel(pidchannel);
      audio_decoder_->SetOnFrameFound(
          std::bind(&MediaPlayer::OnAudioFrameFound, this, _1)
      );

      audio_decoder_->SetOnNullPacketSent(
          std::bind(&MediaPlayer::OnNullPacketSent, this, _1)
      );

      if(audio_renderer_) {
        int err = audio_renderer_->Prepare(
            audio_decoder_->sampleFormat(),
            audio_decoder_->channels(),
            audio_decoder_->samplerate(),
            audio_decoder_->channellayout());
        if(err) {
          LOG(ERROR) << __func__ << " failed to prepare a renderer: " << err;
          return  -1;
        }
      }
    }

    if(source_->HasSubtitle()) {
      int pid = source_->subtitlePid();
      PidChannel *pidchannel = source_->RequestPidChannel(pid);
      if(! pidchannel) {
        LOG(ERROR) << " failed to request a pid-channel for subtitle";
        return -1;
      }

      subtitle_decoder_.reset(new SubtitleDecoder);
      subtitle_decoder_->Prepare(fmt->streams[pid]);
      subtitle_decoder_->SetPidChannel(pidchannel);
      subtitle_decoder_->SetOnSubtitleFound(
          std::bind(&MediaPlayer::OnSubtitleFound, this, _1)
      );

      if(video_renderer_ && subtitle_renderer_) {
        subtitle_renderer_->Prepare(
            video_decoder_->width(),
            video_decoder_->height(),
            subtitle_decoder_->pixelFormat());

        video_renderer_->SetSubtitleRenderer(subtitle_renderer_.get());
      }
    }

    if(on_prepared) on_prepared();

    promise.set_value();
    return 0;
  });

  fut.wait();
  return 0;
}

int MediaPlayer::Start() {
  int err=0;
  if(audio_decoder_) err = audio_decoder_->Start();
  if(err) {
    LOG(WARNING) << " failed to start the audio decoder";
    return err;
  }

  if(video_decoder_) err = video_decoder_->Start();
  if(err) {
    LOG(WARNING) << " failed to start the video decoder";
    return err;
  }

  if(subtitle_decoder_) err = subtitle_decoder_->Start();
  if(err) {
    LOG(WARNING) << " failed to start the subtitle decoder";
    return err;
  }

  err = source_->Start();
  if(err) {
    LOG(WARNING) << " failed to start the demuxer";
    return err;
  }

  SetState(started);

  return 0;
}

int MediaPlayer::Stop() {
  SetState(stopped);

  if(audio_renderer_) {
    audio_renderer_ = nullptr;
  }

  if(audio_decoder_) {
    audio_decoder_->Stop();
    audio_decoder_=nullptr;
  }

  if(video_renderer_) {
    video_renderer_ = nullptr;
  }

  if(video_decoder_) {
    video_decoder_->Stop();
    video_decoder_=nullptr;
  }

  if(subtitle_decoder_) {
    subtitle_decoder_->Stop();
    subtitle_decoder_=nullptr;
  }

  if(source_) {
    source_->Stop();
    source_ = nullptr;
  }

  LOG(INFO) << __func__;
  return 0;
}

int MediaPlayer::Pause() {
  SetState(paused);
  if(source_) source_->Pause();

  if(subtitle_decoder_) subtitle_decoder_->Pause();

  if(audio_decoder_) audio_decoder_->Pause();

  if(video_decoder_) video_decoder_->Pause();

  is_first_frame[SUBTITLE]=is_first_frame[VIDEO]=is_first_frame[AUDIO]=true;

  return 0;
}

void MediaPlayer::SetDataSource(const std::string &datasource) {
  source_->SetDataSource(datasource);
}

void MediaPlayer::OnNullPacketSent(const Decoder &decoder) {
  std::lock_guard<std::mutex> lk(lck_);
  if(eos_sent_) return;

  switch((int)decoder.MediaType()) {
  case AVMEDIA_TYPE_VIDEO:
    LOG(INFO) << " eos of video";
    if(on_end_of_stream_) on_end_of_stream_();
    eos_sent_=true;
    break;

  case AVMEDIA_TYPE_AUDIO:
    LOG(INFO) << " eos of audio";
    if(on_end_of_stream_) on_end_of_stream_();
    eos_sent_=true;
    break;

  case AVMEDIA_TYPE_SUBTITLE:
    LOG(INFO) << " eos of subtitle";
    if(on_end_of_stream_) on_end_of_stream_();
    eos_sent_=true;
    break;

  }
}

void MediaPlayer::OnVideoFrameFound(const AVFrame *frame) {
  if(state()==stopped)
    return;

  if(is_first_frame[VIDEO]) {
    last_pts_[VIDEO] = frame->pts;
    timer_[VIDEO].update();
    is_first_frame[VIDEO]=false;
  }
  else {
    int delay_step=kDefaultDelayStep;
    bool synced=true;
    if(source_->HasAudio()) {
      int diff = last_pts_[VIDEO] - last_pts_[AUDIO] ;
      if(diff > sync_threshold_) { // 1:1000000
        synced=false;
      }
    }

    const int64_t frame_delay = frame->pts - last_pts_[VIDEO];
    last_pts_[VIDEO] = frame->pts;
    timer_[VIDEO].wait(frame_delay + (synced ? 0 : delay_step));
  }

  if(width_ != frame->width || height_ != frame->height) {
    width_ = frame->width;
    height_ = frame->height;
    if(video_renderer_) video_renderer_->Resize(width_, height_);
  }

  if(video_renderer_) video_renderer_->Render(frame);
}

void MediaPlayer::OnSubtitleFound(const AVSubtitle *subtitle) {
  if(state()==stopped)
    return;

  if(is_first_frame[SUBTITLE]) {
    last_pts_[SUBTITLE] = subtitle->pts;
    timer_[SUBTITLE].update();
    is_first_frame[SUBTITLE]=false;
  }
  else {
    bool synced=true;
    int diff;
    if(source_->HasVideo() || source_->HasAudio()) {
      diff = last_pts_[SUBTITLE] - currentPosition();
      if(diff > sync_threshold_) { // 1:1000000
        synced=false;
      }
    }

    const int64_t frame_delay = subtitle->pts - last_pts_[SUBTITLE];
    last_pts_[SUBTITLE] = subtitle->pts;
    timer_[SUBTITLE].wait(frame_delay + (synced ? 0 : diff));
  }
  if(subtitle_renderer_) subtitle_renderer_->Render(subtitle);
}

void MediaPlayer::OnAudioFrameFound(const AVFrame *frame) {
  if(state()==stopped)
    return;

  if(is_first_frame[AUDIO]) {
    last_pts_[AUDIO] = frame->pts;
    timer_[AUDIO].update();
    is_first_frame[AUDIO]=false;
  }
  else {
    int delay_step=kDefaultDelayStep;
    bool synced=true;
    if(source_->HasVideo()) {
      int diff = last_pts_[AUDIO] - last_pts_[VIDEO];
      if(diff > sync_threshold_) { // 1:1000000
        synced=false;
      }
    }

    const int64_t frame_delay = frame->pts - last_pts_[AUDIO];
    last_pts_[AUDIO] = frame->pts;
    timer_[AUDIO].wait(frame_delay + (synced ? 0 : delay_step));
  }

  if(audio_renderer_) audio_renderer_->Render(frame);
}

int64_t MediaPlayer::Seek(uint64_t pos) {
  int flag=(pos < (uint64_t)currentPosition()) ? AVSEEK_FLAG_BACKWARD : 0;
  source_->Seek(pos, flag, [&]{
        if(audio_decoder_) audio_decoder_->Flush();
        if(video_decoder_) video_decoder_->Flush();
        if(subtitle_decoder_) subtitle_decoder_->Flush();

        is_first_frame[VIDEO]=is_first_frame[AUDIO]=is_first_frame[SUBTITLE]=true;
      });

  return 0;
}

int64_t MediaPlayer::duration() {
  return source_->GetDuration();
}

int64_t MediaPlayer::currentPosition() {
  if(source_->HasVideo())
    return last_pts_[VIDEO];

  return last_pts_[AUDIO];
}

void MediaPlayer::SetVideoRenderer(std::unique_ptr<VideoRenderer> renderer) {
  video_renderer_=std::move(renderer);
}

void MediaPlayer::SetAudioRenderer(std::unique_ptr<AudioRenderer> renderer) {
  audio_renderer_=std::move(renderer);
}

void MediaPlayer::SetSubtitleRenderer(std::unique_ptr<SubtitleRenderer> renderer) {
  subtitle_renderer_=std::move(renderer);
}

void MediaPlayer::SetState(State state) {
  std::lock_guard<std::mutex> lk(lck_);
  if(state_!=state) {
    State from = state_;
    state_=state;
    if(on_state_changed_) on_state_changed_(from, state_);
  }
}

void MediaPlayer::SetVolume(float volume) {
  if(audio_renderer_) audio_renderer_->SetVolume(volume);
}

float MediaPlayer::volume() {
  if(audio_renderer_) return audio_renderer_->volume();
  return 0.0f;
}

} // namespace gurum