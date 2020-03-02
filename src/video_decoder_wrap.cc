#include <napi.h>
#include <uv.h>
#include "log_message.h"
#include <future>
#include "frame_wrap.h"
#include "video_decoder_wrap.h"
#include "decode_helper.h"

Napi::FunctionReference VideoDecoder::constructor;

Napi::Object VideoDecoder::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func =
      DefineClass(env,
                  "VideoDecoder",
                  {
                    InstanceMethod("prepare", &VideoDecoder::Prepare),
                    InstanceMethod("start", &VideoDecoder::Start),
                    InstanceMethod("stop", &VideoDecoder::Stop),
                    InstanceMethod("pause", &VideoDecoder::Pause),
                    InstanceMethod("decode", &VideoDecoder::Decode),
                    InstanceMethod("flush", &VideoDecoder::Flush),
                    InstanceAccessor("pidchannel", nullptr, &VideoDecoder::SetPidChannel),
                    InstanceAccessor("width", &VideoDecoder::width, nullptr),
                    InstanceAccessor("height", &VideoDecoder::height, nullptr),
                    InstanceAccessor("pixelformat", &VideoDecoder::pixelFormat, nullptr),
                    InstanceAccessor("trace", &VideoDecoder::log_enabled, &VideoDecoder::EnableLog),
                  });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("VideoDecoder", func);

  return exports;
}

VideoDecoder::VideoDecoder(const Napi::CallbackInfo& info) : Napi::ObjectWrap<VideoDecoder>(info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  decoder_.reset(new gurum::VideoDecoder);
}


void VideoDecoder::SetPidChannel(const Napi::CallbackInfo& info, const Napi::Value &value) {
  DecodeHelper::SetPidChannel(info, value, *decoder_);
}

void VideoDecoder::Prepare(const Napi::CallbackInfo& info) {
  DecodeHelper::Prepare(info, *decoder_);
}

void VideoDecoder::Start(const Napi::CallbackInfo& info) {
  DecodeHelper::Start(info, *decoder_);
}

void VideoDecoder::Stop(const Napi::CallbackInfo& info) {
  DecodeHelper::Stop(info, *decoder_);
}

void VideoDecoder::Pause(const Napi::CallbackInfo& info) {
  DecodeHelper::Pause(info, *decoder_);
}


Napi::Value VideoDecoder::Decode(const Napi::CallbackInfo& info) {
  return DecodeHelper::Decode(info, *decoder_);
}

void VideoDecoder::Flush(const Napi::CallbackInfo& info) {
  if(decoder_) decoder_->Flush();
}

Napi::Value VideoDecoder::width(const Napi::CallbackInfo& info) {
  return Napi::Number::New(info.Env(), decoder_->width());
}

Napi::Value VideoDecoder::height(const Napi::CallbackInfo& info) {
  return Napi::Number::New(info.Env(), decoder_->height());
}

Napi::Value VideoDecoder::pixelFormat(const Napi::CallbackInfo& info) {
  return Napi::Number::New(info.Env(), decoder_->pixelFormat());
}

void VideoDecoder::EnableLog(const Napi::CallbackInfo& info, const Napi::Value &value) {
  Napi::Env env = info.Env();
  if (info.Length() <= 0 || !value.IsBoolean()) {
    Napi::TypeError::New(env, "Boolean expected").ThrowAsJavaScriptException();
    return;
  }
  log_enabled_ = value.ToBoolean();
  if(decoder_) decoder_->EnableLog(log_enabled_);
}

Napi::Value VideoDecoder::log_enabled(const Napi::CallbackInfo& info) {
  return Napi::Boolean::New(info.Env(), log_enabled_);
}