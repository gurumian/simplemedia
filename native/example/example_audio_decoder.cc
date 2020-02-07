#include <simplemedia/config.h>
#include <iostream>
#include <fcntl.h>
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <map>
#include <functional>
#include <simplemedia/sdl/sdl_video_renderer.h>
#include <simplemedia/sdl/sdl_subtitle_renderer.h>
#include <simplemedia/sdl/sdl_audio_renderer.h>
#include "log_message.h"

#include <simplemedia/source.h>
#include <simplemedia/audio_decoder.h>
#include <atomic>
#include <future>

static  void OnRawData(uint8_t* data, size_t size) {
}

static void OnFrameFound(const AVFrame *frame, int channels) {
  int err;

  int data_size = av_get_bytes_per_sample((AVSampleFormat)frame->format);

  for (int i=0; i<frame->nb_samples; i++) {
    for (int ch=0; ch<channels; ch++) {
      OnRawData((frame->data[ch] + data_size*i), data_size);
    }
  }
  return;
}

int main(int argc, char *argv[]) {
  if(argc == 1) {
    LOG(INFO) << __func__ << " no argument";
    return 0;
  }

  std::unique_ptr<gurum::Source> source_{new gurum::Source};
  std::unique_ptr<gurum::AudioDecoder> audio_decoder_;

  source_->EnableLog(true);
  source_->SetDataSource(argv[1]);

  std::promise<void> promise;
  std::future<void> fut = promise.get_future();


  source_->PrepareAsync([&](const AVFormatContext *fmt)->int {
     if(source_->HasAudio()) {
       int pid = source_->audioPid();
       gurum::PidChannel *pidchannel = source_->RequestPidChannel(pid);
       if(! pidchannel) {
         LOG(ERROR) << " failed to request a pid-channel for audio";
         return -1;
       }

       audio_decoder_.reset(new gurum::AudioDecoder);
       audio_decoder_->Prepare(fmt->streams[pid]);
       audio_decoder_->SetPidChannel(pidchannel);
       audio_decoder_->SetOnFrameFound([&](const AVFrame *frame){
         OnFrameFound(frame, audio_decoder_->channels());
       });

       audio_decoder_->SetOnNullPacketSent([&](const gurum::Decoder &decoder){
         if(decoder.MediaType()==AVMEDIA_TYPE_AUDIO) {
           LOG(INFO) << " eos of audio";
           promise.set_value();
         }
       });

       audio_decoder_->Start();
     }

     source_->Start();
     return 0;
  });

  fut.wait();

  audio_decoder_->Stop();
  source_->Stop();
  return 0;
}