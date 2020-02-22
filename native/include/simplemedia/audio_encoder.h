#ifndef GURUM_AUDIO_ENCODER_H_
#define GURUM_AUDIO_ENCODER_H_

#include "encoder.h"

namespace gurum {

class AudioEncoder : public Encoder {
public:
  AudioEncoder()=default;
  virtual ~AudioEncoder()=default;

  AVRational timebase();
  virtual AVMediaType MediaType() override {return AVMEDIA_TYPE_AUDIO;}
};

} // namespace gurum

#endif // GURUM_AUDIO_ENCODER_H_