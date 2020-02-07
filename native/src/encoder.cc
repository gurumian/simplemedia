#include "encoder.h"

namespace gurum {

Encoder::Encoder() {
}

Encoder::~Encoder() {
  if(codec_context_) {
    avcodec_free_context(&codec_context_);
    codec_context_ = nullptr;
  }
}


int Encoder::Prepare(AVStream *stream) {
  // TODO
  return 0;
}

} // namespace gurum