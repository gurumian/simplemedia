#include "simplemedia/config.h"
#include "simplemedia/audio_renderer.h"
#include "log_message.h"

namespace gurum {


std::tuple<gurum::Buffer, int> AudioRenderer::Resample(const AVFrame &frame) {
  uint8_t *data = nullptr;

  int err = av_samples_alloc(&data, NULL, channels_, frame.nb_samples, AV_SAMPLE_FMT_S16, 0);
  if( !data || err < 0) {
    LOG(ERROR) << "failed to av_samples_alloc()";
    return std::make_tuple(nullptr, 0);
  }

  int resample_count = swr_convert(swr_, &data, frame.nb_samples, (const uint8_t **) frame.extended_data, frame.nb_samples);
  if(resample_count <= 0) {
    LOG(ERROR) << "failed to convert" << resample_count;
    return std::make_tuple(nullptr, 0);
  }

  int size = av_samples_get_buffer_size(NULL, channels_, resample_count, AV_SAMPLE_FMT_S16, 0);

  gurum::Buffer out {
    data,
    [](void *ptr){ if(ptr) av_freep(&ptr);}
  };
  return std::make_tuple(std::move(out), size);
}

} // namespace gurum


