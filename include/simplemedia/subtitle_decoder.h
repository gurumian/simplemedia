#ifndef GURUM_SUBTITLE_DECODER_H_
#define GURUM_SUBTITLE_DECODER_H_

#include "decoder.h"

namespace gurum {

class SubtitleDecoder: public Decoder {
public:
  using OnSubtitleFound=std::function<void(const AVSubtitle *subtitle)>;

  virtual ~SubtitleDecoder();

  void SetOnSubtitleFound(OnSubtitleFound on_subtitle_found){on_subtitle_found_=on_subtitle_found;};

  int DidPrepare() override;

  void Run() override;

  int width();
  int height();
  int pixelFormat();
  AVRational timebase();

  virtual AVMediaType MediaType() const override {return AVMEDIA_TYPE_SUBTITLE;}

private:
  int decode(AVPacket *pkt, OnSubtitleFound on_subtitle_found);

  bool log_enabled_=false;
  OnSubtitleFound on_subtitle_found_=nullptr;
  AVSubtitle subtitle_;
};

} // namespace gurum


#endif // GURUM_SUBTITLE_DECODER_H_
