#include <assert.h>
#include "log_message.h"

#include "simplemedia/video_decoder.h"

namespace gurum {

int VideoDecoder::width() {
  return codec_context_->width;
}

int VideoDecoder::height() {
  return codec_context_->height;
}

int VideoDecoder::pixelFormat() {
  return (int)codec_context_->pix_fmt;
}

} // namespace gurum