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

} // namespace gurum


