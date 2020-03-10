#ifndef GURUM_VIDEO_DECODER_WRAP_H
#define GURUM_VIDEO_DECODER_WRAP_H

#include <napi.h>

extern "C" {
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavformat/avformat.h>
}

#include "decoder_wrap.h"
#include "simplemedia/video_decoder.h"


class VideoDecoder : public Decoder<VideoDecoder> {
 public:
  static Napi::Object Init(Napi::Env env, Napi::Object exports);
  VideoDecoder(const Napi::CallbackInfo& info);

private:
  Napi::Value width(const Napi::CallbackInfo& info);
  Napi::Value height(const Napi::CallbackInfo& info);
  Napi::Value pixelFormat(const Napi::CallbackInfo& info);

private:
  static Napi::FunctionReference constructor;
};

#endif // GURUM_VIDEO_DECODER_WRAP_H