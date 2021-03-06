#include "simplemedia/resampler.h"
#include "log_message.h"
#include <cstring>
#include <vector>

namespace gurum {

Resampler::Resampler(const AudioSettings &in, const AudioSettings &out)
: in_(in), out_(out) {

#if defined(USE_SWRESAMPLE)
  swr_ = swr_alloc();
  assert(swr_);
  av_opt_set_int(swr_, "in_channel_count", in.channels, 0);
  av_opt_set_int(swr_, "in_channel_layout", in.channellayout, 0);
  av_opt_set_int(swr_, "in_sample_rate", in.samplerate, 0);
  av_opt_set_sample_fmt(swr_, "in_sample_fmt", in.sampleformat, 0);

  av_opt_set_int(swr_, "out_channel_count", out.channels, 0);
  av_opt_set_int(swr_, "out_channel_layout", out.channellayout, 0);
  av_opt_set_int(swr_, "out_sample_rate", out.samplerate, 0);
  av_opt_set_sample_fmt(swr_, "out_sample_fmt", out.sampleformat,  0);

  int err = swr_init(swr_);
  if(err) {
    LOG(FATAL) << " failed to swr_init";
  }
#endif
}

Resampler::~Resampler() {
#if defined(USE_SWRESAMPLE)
  if(swr_) {
    swr_close(swr_);
    swr_free(&swr_);
  }
#endif // USE_SWRESAMPLE
}

std::tuple<gurum::Buffer, int> Resampler::Resample(const AVFrame &frame) {
#if defined(USE_SWRESAMPLE)
  uint8_t *data = nullptr;

  int err = av_samples_alloc(&data, NULL, out_.channels, frame.nb_samples, out_.sampleformat, 0);
  if( !data || err < 0) {
    LOG(ERROR) << "failed to av_samples_alloc()";
    return std::make_tuple(nullptr, 0);
  }

  int resample_count = swr_convert(swr_, &data, frame.nb_samples, (const uint8_t **) frame.extended_data, frame.nb_samples);
  if(resample_count <= 0) {
    LOG(ERROR) << "failed to convert" << resample_count;
    return std::make_tuple(nullptr, 0);
  }

  int size = av_samples_get_buffer_size(NULL, out_.channels, resample_count, out_.sampleformat, 0);

  gurum::Buffer out {
    data,
    [](void *ptr){ if(ptr) av_freep(&ptr);}
  };
  return std::make_tuple(std::move(out), size);
#else
  assert(1);
#endif // USE_SWRESAMPLE
}

}