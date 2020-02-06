#ifndef GURUM_FRAME_DECODER_H_
#define GURUM_FRAME_DECODER_H_

#include "decoder.h"

namespace gurum {

class FrameDecoder: public Decoder {
public:
  using OnFrameFound=std::function<void(const AVFrame *frame)>;

  virtual ~FrameDecoder();

  void SetOnFrameFound(OnFrameFound on_frame_found){on_frame_found_=on_frame_found;};

  int DidPrepare() override;

  void Run() override;

  AVFrame *frame() { return frame_; }

  int Decode(OnFrameFound on_frame_found);

protected:
  virtual int decode(AVPacket *pkt, OnFrameFound on_frame_found)=0;

  OnFrameFound on_frame_found_=nullptr;
  AVFrame *frame_=nullptr;
};

} // namespace gurum

#endif // GURUM_FRAME_DECODER_H_
