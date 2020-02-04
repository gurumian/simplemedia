#include <napi.h>
#include <uv.h>
#include "frame_wrap.h"
#include "log_message.h"

Napi::FunctionReference Frame::constructor;

Napi::Object Frame::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func =
      DefineClass(env,
                  "Frame",
                  {
                    InstanceAccessor("pts", &Frame::pts, nullptr),
                    InstanceAccessor("data", &Frame::data, nullptr),
                  });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("Frame", func);

  return exports;
}

Frame::Frame(const Napi::CallbackInfo& info) : Napi::ObjectWrap<Frame>(info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  if (info.Length() <= 0 || !info[0].IsExternal()) {
    Napi::TypeError::New(env, "External expected").ThrowAsJavaScriptException();
    return;
  }

  auto frame = info[0].As<Napi::External<AVFrame>>();
  frame_ = frame.Data();
}

Napi::Object Frame::NewInstance(Napi::Env env, Napi::Value arg) {
  Napi::EscapableHandleScope scope(env);
  Napi::Object obj = constructor.New({arg});
  return scope.Escape(napi_value(obj)).ToObject();
}

Napi::Value Frame::pts(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  return Napi::Number::New(env, frame_->pts);
}

Napi::Value Frame::data(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  return Napi::External<AVFrame>::New(env, frame_);
}