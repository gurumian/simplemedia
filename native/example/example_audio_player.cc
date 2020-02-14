#include <iostream>
#include <fcntl.h>
#include <assert.h>
#include <stdio.h>
#include <simplemedia/sdl/sdl_audio_renderer.h>
#include "log_message.h"
#include "simplemedia/source.h"
#include "simplemedia/audio_decoder.h"

using namespace gurum;

int main(int argc, char *argv[]) {
  int err = 0;
  std::string uri = argv[1];

  bool is_first_frame = true;
  Timer timer;
  int64_t last_pts;

  std::unique_ptr<SdlAudioRenderer> audio_renderer{ new SdlAudioRenderer };

  std::unique_ptr<Source> source{new Source};
  std::unique_ptr<AudioDecoder> audio_decoder;
  source->EnableLog(true);
  source->SetDataSource(uri);
  source->PrepareAsync([&](const AVFormatContext *fmt)->int {
    if(source->HasAudio()) {
      int pid = source->audioPid();
      AVStream *strm = source->FindStream(pid);

      PidChannel *pidchannel = source->RequestPidChannel(pid);
      if(! pidchannel) {
        LOG(ERROR) << " failed to request a pid-channel for audio";
        return -1;
      }

      audio_decoder.reset(new AudioDecoder);
      audio_decoder->EnableLog(true);
      audio_decoder->Prepare(strm);
      audio_decoder->SetPidChannel(pidchannel);
      audio_decoder->SetOnFrameFound([&](const AVFrame *frame) {
        if(is_first_frame) {
          last_pts = frame->pts;
          timer.update();
          is_first_frame=false;
        }
        else {
          const int64_t frame_delay = frame->pts - last_pts;
          last_pts = frame->pts;
          timer.wait(frame_delay);
        }

        if(audio_renderer) audio_renderer->Render(frame);
      });

      audio_decoder->SetOnNullPacketSent([&](const Decoder &decoder) {
        LOG(INFO) << " eos of audio";
      });

      if(audio_renderer) {
        audio_renderer->EnableLog(true);
        int err = audio_renderer->Prepare(
            audio_decoder->sampleFormat(),
            audio_decoder->channels(),
            audio_decoder->samplerate(),
            audio_decoder->channellayout());
      }

      audio_decoder->Start();
    }
    
    source->Start();
    return 0;
  });

  getchar();

  audio_renderer = nullptr;

  audio_decoder->Stop();
  audio_decoder=nullptr;

  source->Stop();
  source = nullptr;
  LOG(INFO) << __func__ << " done";
  return 0;
}