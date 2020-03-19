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

std::unique_ptr<gurum::Resampler> AudioDecoder::CreateResampler(const gurum::AudioSettings &settings) {
  gurum::AudioSettings in{};
  in.sampleformat = sampleFormat();
  in.samplerate = (int64_t)samplerate();
  in.channellayout = channellayout();
  in.channels = channels();
  std::unique_ptr<gurum::Resampler> resampler{new gurum::Resampler(in, settings)};
  return resampler;
}

} // namespace gurum