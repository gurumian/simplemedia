#include "simplemedia/config.h"
#include "simplemedia/audio_renderer.h"
#include "log_message.h"

namespace gurum {

std::tuple<gurum::Buffer, int> AudioRenderer::Resample(const AVFrame &frame) {
  if(!resampler_) {
    resampler_.reset(new Resampler(fmt_, channels_, samplerate_, channel_layout_));
  }
  return resampler_->Resample(frame);
}

int AudioRenderer::Prepare(const gurum::AudioSettings &settings) {
  return Prepare(settings.sampleformat, settings.channels, (int)settings.samplerate, settings.channellayout);
}

} // namespace gurum


