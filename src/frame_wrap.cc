#include <napi.h>
#include <uv.h>
#include "frame_wrap.h"
#include "log_message.h"

Napi::FunctionReference Frame::constructor;

Napi::Object Frame::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  constexpr auto name = "Frame";

  Napi::Function func =
      DefineClass(env,
                  name,
                  {
                    InstanceAccessor("pts", &Frame::pts, nullptr),
                    InstanceAccessor("native", &Frame::native, nullptr),
                    InstanceAccessor("data", &Frame::data, nullptr),
                    InstanceAccessor("numofSamples", &Frame::nb_samples, nullptr),
                    InstanceAccessor("bytesPerSample", &Frame::bytes_per_sample, nullptr),
                  });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set(name, func);

  return exports;
}

Frame::Frame(const Napi::CallbackInfo& info) : Napi::ObjectWrap<Frame>(info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  if (info.Length() <= 0 || !info[0].IsExternal()) {
    Napi::TypeError::New(env, "External expected").ThrowAsJavaScriptException();
    return;
  }

  auto frame = info[0].As<Napi::External<AVFrame>>().Data();
  int err{0};
  frame_ = av_frame_alloc();
  frame_->format = frame->format;
  frame_->width = frame->width;
  frame_->height = frame->height;
  frame_->channels = frame->channels;
  frame_->channel_layout = frame->channel_layout;
  frame_->nb_samples = frame->nb_samples;
  err = av_frame_get_buffer(frame_, 32);
  assert(!err);
  err = av_frame_copy(frame_, frame);
  assert(!err);
  err = av_frame_copy_props(frame_, frame);
  assert(!err);
}

Frame::~Frame() {
  if(frame_) {
    av_frame_free(&frame_);
    frame_ = nullptr;
  }
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

Napi::Value Frame::native(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  return Napi::External<AVFrame>::New(env, frame_);
}

Napi::Value Frame::data(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if(frame_->channels) {
    int data_size_per_sample = av_get_bytes_per_sample((AVSampleFormat)frame_->format);
    auto data = Napi::Array::New(env, frame_->nb_samples);
    for(int i = 0; i < frame_->nb_samples; i++) {
      auto sub = Napi::Array::New(env, frame_->channels);
      for(int ch = 0; ch < frame_->channels; ch++) {
        sub[ch] = Napi::ArrayBuffer::New(env, (frame_->data[ch] + data_size_per_sample*i), data_size_per_sample);
      }
      data[i] = sub;
    }
    return data;
  }
  else {
    const int len = 3;
    auto data = Napi::Array::New(env, len);
    for(int i = 0; i < len; i++) {
      data[i] = Napi::ArrayBuffer::New(env, frame_->data[i], frame_->linesize[i]);
    }
    return data;
  }
  Napi::TypeError::New(env, "unhandled").ThrowAsJavaScriptException();
  return env.Undefined();
}

Napi::Value Frame::nb_samples(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  return Napi::Number::New(env, frame_->nb_samples);
}

Napi::Value Frame::bytes_per_sample(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  return Napi::Number::New(env, av_get_bytes_per_sample((AVSampleFormat)frame_->format));
}