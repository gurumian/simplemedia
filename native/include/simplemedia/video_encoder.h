#ifndef GURUM_VIDEO_ENCODER_H_
#define GURUM_VIDEO_ENCODER_H_

#include "encoder.h"

namespace gurum {

class VideoEncoder : public Encoder {
public:
  VideoEncoder()=default;
  virtual ~VideoEncoder()=default;

  virtual AVMediaType MediaType() override {return AVMEDIA_TYPE_VIDEO;}

private:

};

} // namespace gurum

#endif // GURUM_VIDEO_ENCODER_H_