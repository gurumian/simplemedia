#include "simplemedia/config.h"
#include "simplemedia/audio_decoder.h"
#include "log_message.h"

namespace gurum {

int AudioDecoder::samplerate() {
  assert(codec_context_);
  return codec_context_->sample_rate;
}

AVSampleFormat AudioDecoder::sampleFormat() {
  assert(codec_context_);
  return codec_context_->sample_fmt;
}

int AudioDecoder::channels() {
  assert(codec_context_);
  return codec_context_->channels;
}

int64_t AudioDecoder::channellayout() {
  assert(codec_context_);
  return codec_context_->channel_layout;
}

} // namespace gurum