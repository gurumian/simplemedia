#ifndef GURUM_ENCODER_H_
#define GURUM_ENCODER_H_

#include "simplemedia/config.h"
#include <assert.h>
#include <memory>
#include <algorithm>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stddef.h>
#include <iostream>

extern "C" {
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include <libavutil/avstring.h>
#include <libavutil/eval.h>
#include <libavutil/mathematics.h>
#include <libavutil/pixdesc.h>
#include <libavutil/imgutils.h>
#include <libavutil/dict.h>
#include <libavutil/parseutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/avassert.h>
#include <libavutil/time.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavcodec/avfft.h>
}
#include <assert.h>
#include <functional>

namespace gurum {

class Encoder {
public:
  using OnWillPrepare=std::function<int()>;
  using OnPrepared=std::function<int()>;
  using OnPacketFound=std::function<void(AVPacket *pkt)>;

  Encoder();
  virtual ~Encoder();

  int Prepare(AVStream *stream);

  void SetOnWillPrepare(OnWillPrepare on_will_prepare){on_will_prepare_=on_will_prepare;}
  void SetOnPrepared(OnPrepared on_prepared){on_prepared_=on_prepared;}

  virtual int Encode(AVFrame *frame, OnPacketFound on_packet_found)=0;
  virtual AVMediaType MediaType()=0;

protected:
  AVCodecContext *codec_context_=nullptr;
  AVCodec *codec_=nullptr;

  OnWillPrepare on_will_prepare_=nullptr;
  OnPrepared on_prepared_=nullptr;
};

} // namespace gurum

#endif // GURUM_ENCODER_H_
