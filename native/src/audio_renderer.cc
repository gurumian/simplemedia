#include "simplemedia/config.h"
#include "simplemedia/audio_renderer.h"
#include "log_message.h"
#include "simplemedia/types.h"

namespace gurum {

std::tuple<gurum::Buffer, int> AudioRenderer::Resample(const AVFrame &frame) {
  // TODO:
  if(!resampler_) {
    gurum::AudioSettings in{
      fmt_,
      channel_layout_,
      samplerate_,
      channels_,
    };
    
    gurum::AudioSettings out = in;
    out.sampleformat = AV_SAMPLE_FMT_S16;
    resampler_.reset(new Resampler(in, out));
  }
  return resampler_->Resample(frame);
}

int AudioRenderer::Prepare(const gurum::AudioSettings &settings) {
  return Prepare(settings.sampleformat, settings.channels, (int)settings.samplerate, settings.channellayout);
}

} // namespace gurum


