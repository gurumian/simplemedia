#ifndef GURUM_AUDIO_DECODER_H_
#define GURUM_AUDIO_DECODER_H_

#include "frame_decoder.h"
#include "simplemedia/types.h"
#include "simplemedia/resampler.h"

namespace gurum {

class AudioDecoder: public FrameDecoder {
public:
  virtual AVMediaType MediaType() const override {return AVMEDIA_TYPE_AUDIO;}

  int samplerate();
  AVSampleFormat sampleFormat();
  int channels();
  int64_t channellayout();

  std::unique_ptr<gurum::Resampler> CreateResampler(const gurum::AudioSettings &settings);
};

} // namespace gurum

#endif // GURUM_AUDIO_DECODER_H_