#include <napi.h>
#include <uv.h>
#include "frame_wrap.h"
#include "log_message.h"
#include <algorithm>
#include <iterator>
#include "simplemedia/types.h"
extern "C" {
#include <libavutil/base64.h>
}

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
                    InstanceAccessor("base64", &Frame::base64, nullptr),
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

bool Frame::isAudio() {
  return (frame_->channels > 0);
}

std::tuple<gurum::Buffer, int> Frame::data(const AVFrame &frame) {
  if(isAudio()) {
    std::vector<uint8_t> chunk;
    int size = av_get_bytes_per_sample((AVSampleFormat)frame_->format);
    for(int i = 0; i < frame_->nb_samples; i++) {
      for(int ch = 0; ch < frame_->channels; ch++) {
        uint8_t *ptr = (frame_->data[ch] + size*i);
        chunk.insert(chunk.end(), ptr, ptr+size);
      }
    }

    gurum::Buffer out {
      (uint8_t *) av_malloc(chunk.size()),
      [](void *ptr){ if(ptr) av_freep(&ptr);}
    };
    return std::make_tuple(std::move(out), chunk.size());
  }
  else {
    // TODO:
  }

  return std::make_tuple(nullptr, 0);
}

Napi::Value Frame::data(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if(isAudio()) {
    gurum::Buffer lineared{};
    int size;
    std::tie(lineared, size) = data(*frame_);
    return Napi::ArrayBuffer::New(env, lineared.get(), size);
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

Napi::Value Frame::base64(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if(isAudio()) {
    std::vector<uint8_t> chunk;
    gurum::Buffer lineared{};
    int size;
    std::tie(lineared, size) = data(*frame_);

    int b64_size = AV_BASE64_SIZE(size);
    gurum::Buffer buf {
      (uint8_t *)av_malloc(b64_size),
      [](void *ptr) { if(ptr) av_freep(&ptr);}
    };

    av_base64_encode((char *)buf.get(), b64_size, lineared.get(), size);

    return Napi::String::New(env, (char *)buf.get());
  }
  else {
    // TODO:
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