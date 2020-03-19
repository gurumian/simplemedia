#include "simplemedia/config.h"
#include "simplemedia/audio_renderer.h"
#include "log_message.h"
#include "simplemedia/types.h"
#include <vector>

namespace gurum {

std::tuple<gurum::Buffer, int> AudioRenderer::Resample(const AVFrame &frame) {
  if(resampler_) {
    return resampler_->Resample(frame);
  }

  std::vector<uint8_t> chunk;
  int data_size = av_get_bytes_per_sample((AVSampleFormat)frame.format);
  for(int i = 0; i < frame.nb_samples; i++) {
    for(int ch = 0; ch < frame.channels; ch++) {
      uint8_t *ptr = frame.data[ch] + data_size * i;
      chunk.insert(chunk.end(), ptr, ptr+data_size);
    }
  }

  int size = chunk.size();
  gurum::Buffer out {
    (uint8_t *)malloc(size),
    [](void *ptr){ if(ptr) free(ptr);}
  };

  std::copy(chunk.begin(), chunk.end(), out.get());
  return std::make_tuple(std::move(out), size);
}

int AudioRenderer::Prepare(const gurum::AudioSettings &settings) {
  return Prepare(settings.sampleformat, settings.channels, (int)settings.samplerate, settings.channellayout);
}

} // namespace gurum