#ifndef GURUM_FRAME_WRAP_H
#define GURUM_FRAME_WRAP_H

#include <napi.h>

extern "C" {
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
}

class Frame : public Napi::ObjectWrap<Frame> {
 public:
  static Napi::Object Init(Napi::Env env, Napi::Object exports);
  Frame(const Napi::CallbackInfo& info);

  static Napi::Object NewInstance(Napi::Env env, Napi::Value arg);

private:
  Napi::Value pts(const Napi::CallbackInfo& info);
  Napi::Value data(const Napi::CallbackInfo& info);

private:
  static Napi::FunctionReference constructor;

  AVFrame *frame_{nullptr};
};

#endif // GURUM_FRAME_WRAP_H


