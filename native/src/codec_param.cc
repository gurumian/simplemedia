#include "simplemedia/config.h"
#include "simplemedia/codec_param.h"
#include "log_message.h"

namespace gurum {

CodecParam::CodecParam() {
  codecpar = avcodec_parameters_alloc();
}

CodecParam::CodecParam(const CodecParam &rhs): CodecParam(rhs.codecpar, rhs.timebase) {}

CodecParam::CodecParam(const AVCodecParameters *arg_codecpar, const AVRational &arg_timebase): CodecParam(){
  avcodec_parameters_copy (codecpar, arg_codecpar);
  timebase = arg_timebase;
}

CodecParam::CodecParam(const AVStream *strm) 
: CodecParam((const AVCodecParameters *) strm->codecpar, strm->time_base){}

CodecParam::~CodecParam() {
  if(codecpar)
   avcodec_parameters_free(&codecpar);
  codecpar = nullptr;
}

} // namespace gurum