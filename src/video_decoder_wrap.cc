#include <napi.h>
#include <uv.h>
#include "log_message.h"
#include <future>
#include "frame_wrap.h"
#include "video_decoder_wrap.h"

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

VideoDecoder::VideoDecoder(const Napi::CallbackInfo& info) : Decoder<VideoDecoder>(info) {
  std::unique_ptr<gurum::VideoDecoder> decoder{new gurum::VideoDecoder};
  decoder_ = std::move(decoder);
}

Napi::Value VideoDecoder::width(const Napi::CallbackInfo& info) {
  auto decoder = static_cast<gurum::VideoDecoder *>(decoder_.get());
  return Napi::Number::New(info.Env(), decoder->width());
}

Napi::Value VideoDecoder::height(const Napi::CallbackInfo& info) {
  auto decoder = static_cast<gurum::VideoDecoder *>(decoder_.get());
  return Napi::Number::New(info.Env(), decoder->height());
}

Napi::Value VideoDecoder::pixelFormat(const Napi::CallbackInfo& info) {
  auto decoder = static_cast<gurum::VideoDecoder *>(decoder_.get());
  return Napi::Number::New(info.Env(), decoder->pixelFormat());
}