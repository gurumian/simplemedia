#ifndef GURUM_VIDEO_DECODER_WRAP_H
#define GURUM_VIDEO_DECODER_WRAP_H

#include <napi.h>

extern "C" {
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
}

#include "simplemedia/video_decoder.h"


class VideoDecoder : public Napi::ObjectWrap<VideoDecoder> {
 public:
  static Napi::Object Init(Napi::Env env, Napi::Object exports);
  VideoDecoder(const Napi::CallbackInfo& info);


private:
  void Prepare(const Napi::CallbackInfo& info);
  void Start(const Napi::CallbackInfo& info);
  void Stop(const Napi::CallbackInfo& info);
  void Pause(const Napi::CallbackInfo& info);

  void Decode(const Napi::CallbackInfo& info);


  void SetPidChannel(const Napi::CallbackInfo& info, const Napi::Value &value);

  Napi::Value width(const Napi::CallbackInfo& info);
  Napi::Value height(const Napi::CallbackInfo& info);
  Napi::Value pixelFormat(const Napi::CallbackInfo& info);


  void EnableLog(const Napi::CallbackInfo& info, const Napi::Value &value);
  Napi::Value log_enabled(const Napi::CallbackInfo& info);

private:
  static Napi::FunctionReference constructor;

  std::unique_ptr<gurum::VideoDecoder> decoder_{};
  bool log_enabled_{false};
};

#endif // GURUM_VIDEO_DECODER_WRAP_H


