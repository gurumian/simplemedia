#ifndef GURUM_SWR_WRAPPER_H_
#define GURUM_SWR_WRAPPER_H_

#include "simplemedia/config.h"

extern "C" {
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#if defined(USE_SWRESAMPLE)
#include <libswresample/swresample.h>
#endif
}

#include <tuple>
#include <functional>
#include "simplemedia/types.h"

namespace gurum {

class Resampler {
public:
  explicit Resampler(AVSampleFormat fmt, int channels, int samplerate, int64_t channel_layout);
  ~Resampler();

  std::tuple<gurum::Buffer, int> Resample(const AVFrame &frame);

private:
#if defined(USE_SWRESAMPLE)
  struct SwrContext *swr_{nullptr};
#endif
  AVSampleFormat fmt_{AV_SAMPLE_FMT_NONE};
  int channels_{0};
  int samplerate_{0};
  int64_t channel_layout_{0};
};

}

#endif // GURUM_SWR_WRAPPER_H_