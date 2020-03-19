#include "resampler_wrap.h"
#include <napi.h>
#include <uv.h>
#include "log_message.h"


static
gurum::AudioSettings ObjectToAudioSetting(Napi::Env env, Napi::Object arg) {
  Napi::Object obj = arg.ToObject();
  if(!obj.HasOwnProperty("sampleformat")) {
    Napi::TypeError::New(env, "no sampleformat").ThrowAsJavaScriptException();
    return {};
  }

  Napi::Value value = obj["sampleformat"];
  if(! value.IsNumber()) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
    return {};
  }

  int sampleformat = (int)value.ToNumber();


  if(!obj.HasOwnProperty("channels")) {
    Napi::TypeError::New(env, "no channels").ThrowAsJavaScriptException();
    return {};
  }

  value = obj["channels"];
  if(! value.IsNumber()) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
    return {};
  }

  int channels = (int) value.ToNumber();


  if(!obj.HasOwnProperty("samplerate")) {
    Napi::TypeError::New(env, "no samplerate").ThrowAsJavaScriptException();
    return {};
  }
  
  value = obj["samplerate"];
  if(! value.IsNumber()) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
    return {};
  }

  int64_t samplerate = (int64_t) value.ToNumber();



  if(!obj.HasOwnProperty("channellayout")) {
    Napi::TypeError::New(env, "no channellayout").ThrowAsJavaScriptException();
    return {};
  }

  value = obj["channellayout"];
  if(! value.IsNumber()) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
    return {};
  }

  int64_t channellayout = (int64_t) value.ToNumber();
  
  gurum::AudioSettings settings{};
  settings.sampleformat = (AVSampleFormat) sampleformat;
  settings.channels = channels;
  settings.samplerate = samplerate;
  settings.channellayout = channellayout;
  return settings;
}

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

  gurum::AudioSettings src, dst;

  if (info.Length() <= 0 || !info[0].IsObject()) {
    Napi::TypeError::New(env, "Object expected").ThrowAsJavaScriptException();
    return;
  }

  Napi::Object obj = info[0].ToObject();
  {
    if(!obj.HasOwnProperty("src")) {
      Napi::TypeError::New(env, "no src").ThrowAsJavaScriptException();
      return;
    }

    Napi::Value value = obj["src"];
    if(! value.IsObject()) {
      Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
      return;
    }
    src = ObjectToAudioSetting(env, value.ToObject());
  }

  {
    if(!obj.HasOwnProperty("dst")) {
      Napi::TypeError::New(env, "no dst").ThrowAsJavaScriptException();
      return;
    }

    Napi::Value value = obj["dst"];
    if(! value.IsObject()) {
      Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
      return;
    }
    dst = ObjectToAudioSetting(env, value.ToObject());
  }

  resampler_.reset(new gurum::Resampler(src, dst));
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