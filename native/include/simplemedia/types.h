#ifndef GURUM_TYPES_H_
#define GURUM_TYPES_H_

#include <memory>
#include <functional>
#include <libavcodec/avcodec.h>

namespace gurum {

using CodecParameters=std::unique_ptr<AVCodecParameters, std::function<void(AVCodecParameters *)>>;
using Buffer=std::unique_ptr<uint8_t, std::function<void(void *)>>;

struct AudioSettings {
  AVSampleFormat sampleformat;
  int64_t channellayout;
  int64_t samplerate;
  int channels;
};

}


#endif // GURUM_TYPES_H_