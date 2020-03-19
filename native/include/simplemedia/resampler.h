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
  explicit Resampler(const AudioSettings &in, const AudioSettings &out);
  ~Resampler();

  std::tuple<gurum::Buffer, int> Resample(const AVFrame &frame);

private:
#if defined(USE_SWRESAMPLE)
  struct SwrContext *swr_{nullptr};
#endif
  AudioSettings in_{};
  AudioSettings out_{};
};

}

#endif // GURUM_SWR_WRAPPER_H_