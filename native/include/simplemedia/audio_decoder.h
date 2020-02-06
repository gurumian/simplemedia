#ifndef GURUM_AUDIO_DECODER_H_
#define GURUM_AUDIO_DECODER_H_

#include "frame_decoder.h"

namespace gurum {

class AudioDecoder: public FrameDecoder {
public:
  AVRational timebase();

  virtual AVMediaType MediaType() const override {return AVMEDIA_TYPE_AUDIO;}

  int samplerate();
  AVSampleFormat sampleFormat();
  int channels();
  int64_t channellayout();

private:
  int decode(AVPacket *pkt, OnFrameFound on_frame_found) override;
};

} // namespace gurum

#endif // GURUM_AUDIO_DECODER_H_
