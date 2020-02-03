#ifndef GURUM_AUDIO_ENCODER_H_
#define GURUM_AUDIO_ENCODER_H_

#include "encoder.h"

namespace gurum {

class AudioEncoder : public Encoder {
public:
  AudioEncoder();
  virtual ~AudioEncoder();

  int Encode(AVFrame *frame, OnPacketFound on_packet_found) override;

  AVRational timebase();

  virtual AVMediaType MediaType() override {return AVMEDIA_TYPE_AUDIO;}

  int samplerate();
  int sampleFormat();
  int channels();
  int64_t channellayout();
};

} // namespace gurum

#endif // GURUM_AUDIO_ENCODER_H_
