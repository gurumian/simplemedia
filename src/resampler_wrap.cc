#include "resampler_wrap.h"
#include <napi.h>
#include <uv.h>
#include "log_message.h"

Napi::FunctionReference Resampler::constructor;

Napi::Object Resampler::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  constexpr auto name = "_Resampler";

  Napi::Function func =
      DefineClass(env,
                  name,
                  {
                    InstanceMethod("resample", &Resampler::resample),
                  });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set(name, func);

  return exports;
}

Resampler::Resampler(const Napi::CallbackInfo& info) : Napi::ObjectWrap<Resampler>(info) {
  if(log_enabled_) LOG(INFO) << __func__;
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  if (info.Length() <= 0 || !info[0].IsObject()) {
    Napi::TypeError::New(env, "Object expected").ThrowAsJavaScriptException();
    return;
  }

  Napi::Object obj = info[0].ToObject();
  if(!obj.HasOwnProperty("sampleformat")) {
    Napi::TypeError::New(env, "no sampleformat").ThrowAsJavaScriptException();
    return;
  }

  Napi::Value value = obj["sampleformat"];
  if(! value.IsNumber()) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
    return;
  }

  int sampleformat = (int)value.ToNumber();


  if(!obj.HasOwnProperty("channels")) {
    Napi::TypeError::New(env, "no channels").ThrowAsJavaScriptException();
    return;
  }

  value = obj["channels"];
  if(! value.IsNumber()) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
    return;
  }

  int channels = (int) value.ToNumber();


  if(!obj.HasOwnProperty("samplerate")) {
    Napi::TypeError::New(env, "no samplerate").ThrowAsJavaScriptException();
    return;
  }
  
  value = obj["samplerate"];
  if(! value.IsNumber()) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
    return;
  }

  int64_t samplerate = (int64_t) value.ToNumber();



  if(!obj.HasOwnProperty("channellayout")) {
    Napi::TypeError::New(env, "no channellayout").ThrowAsJavaScriptException();
    return;
  }

  value = obj["channellayout"];
  if(! value.IsNumber()) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
    return;
  }

  int64_t channellayout = (int64_t) value.ToNumber();
  
  resampler_.reset(new gurum::Resampler(
    (AVSampleFormat) sampleformat, channels, samplerate, channellayout
  ));
}

Napi::Value Resampler::resample(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (info.Length() <= 0 || !info[0].IsExternal()) {
    Napi::TypeError::New(info.Env(), "External expected").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  auto external = info[0].As<Napi::External<AVFrame>>();
  auto frame = external.Data();

  gurum::Buffer resampled{};
  int size;
  std::tie(resampled, size) = resampler_->Resample(*frame);

  auto data = Napi::Array::New(env, size);
  return Napi::ArrayBuffer::New(env, resampled.get(), size);
}