#ifndef GURUM_MEDIA_PLAYER_H_
#define GURUM_MEDIA_PLAYER_H_

#include "simplemedia/config.h"
#include <string>
#include "simplemedia/pid_channel.h"
#include "simplemedia/source.h"
#include "simplemedia/audio_decoder.h"
#include "simplemedia/video_decoder.h"
#include "simplemedia/subtitle_decoder.h"

#include "simplemedia/audio_renderer.h"
#include "simplemedia/video_renderer.h"
#include "simplemedia/subtitle_renderer.h"
#include <functional>
#include <atomic>

namespace base {
class SimpleThread;
}

namespace gurum {

class MediaPlayer {
public:
  using OnEndOfStream=std::function<void()>;
  using OnStateChanged=std::function<void(gurum::State from, gurum::State to)>;
  using OnPrepared=std::function<void()>;

  MediaPlayer();
  virtual ~MediaPlayer();

  int Prepare(OnPrepared on_prepared=nullptr);
  int Start();
  int Stop();
  int Pause();
  int Resume(){ return Start();}

  int64_t Seek(uint64_t pos);

  void SetDataSource(const std::string &datasource);
  void SetVideoRenderer(std::unique_ptr<VideoRenderer> renderer);
  void SetAudioRenderer(std::unique_ptr<AudioRenderer> renderer);
  void SetSubtitleRenderer(std::unique_ptr<SubtitleRenderer> renderer);
  void SetOnEndOfStream(OnEndOfStream on_end_of_stream){ on_end_of_stream_=on_end_of_stream;}
  void SetOnStateChanged(OnStateChanged on_state_changed){ on_state_changed_=on_state_changed;}

  State state() { return state_; }
  int64_t duration();
  int64_t currentPosition();

  // 0.0 ~ 1.0
  void SetVolume(float volume);
  float volume();

  void SetSyncThreshold(int threshold) { sync_threshold_=threshold;}

  void EnableLog(bool enable=true) { log_enabled_=enable;}


  VideoRenderer *videoRenderer() {
    return (video_renderer_) ? video_renderer_.get() : nullptr;
  };

private:
  void SetState(State state);

private:
  void OnVideoFrameFound(const AVFrame *frame);
  void OnAudioFrameFound(const AVFrame *frame);
  void OnSubtitleFound(const AVSubtitle *subtitle);
  void OnNullPacketSent(const Decoder &decoder);

private:
  static constexpr int kDefaultSyncThreshold{5000};
  static constexpr int kDefaultDelayStep{100};

  std::mutex lck_;
  std::unique_ptr<Source> source_{new Source};

  std::unique_ptr<VideoDecoder> video_decoder_;
  std::unique_ptr<AudioDecoder> audio_decoder_;
  std::unique_ptr<SubtitleDecoder> subtitle_decoder_;

  std::unique_ptr<VideoRenderer> video_renderer_;
  std::unique_ptr<AudioRenderer> audio_renderer_;
  std::unique_ptr<SubtitleRenderer> subtitle_renderer_;

  Timer timer_[NUMOF_STREAM_TYPE];
  int64_t last_pts_[NUMOF_STREAM_TYPE] = {0, 0, 0};
  bool is_first_frame[NUMOF_STREAM_TYPE] = {true, true, true};
  std::atomic<State> state_{none};

  int width_{0};
  int height_{0};

  std::atomic<bool> eos_sent_{false};
  OnEndOfStream on_end_of_stream_{nullptr};
  OnStateChanged on_state_changed_{nullptr};
  int sync_threshold_{0};
  bool log_enabled_{false};
};

} // namespace gurum

#endif // GURUM_MEDIA_PLAYER_H_