#ifndef GURUM_CODEC_PARAMETERS_H_
#define GURUM_CODEC_PARAMETERS_H_

#include <memory>
#include <functional>
extern "C" {
#include <libavformat/avformat.h>
}

namespace gurum {

struct CodecParam {
  CodecParam();
  CodecParam(const CodecParam &rhs);
  CodecParam(const AVStream *strm);
  CodecParam(const AVCodecParameters *arg_codecpar, const AVRational &arg_timebase);
  ~CodecParam();

  AVCodecParameters *codecpar{};
  AVRational timebase{};
};

}


#endif // GURUM_CODEC_PARAMETERS_H_