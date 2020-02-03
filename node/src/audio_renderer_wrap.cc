#include "audio_renderer_wrap.h"
#include <napi.h>
#include <uv.h>
#include "log_message.h"
#include <unistd.h>

Napi::FunctionReference AudioRenderer::constructor;

Napi::Object AudioRenderer::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func =
      DefineClass(env,
                  "AudioRenderer",
                  {
                    InstanceMethod("prepare", &AudioRenderer::Prepare),
                    InstanceMethod("render", &AudioRenderer::Render),
                    InstanceAccessor("trace", &AudioRenderer::log_enabled, &AudioRenderer::EnableLog),
                  });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("AudioRenderer", func);

  return exports;
}

AudioRenderer::AudioRenderer(const Napi::CallbackInfo& info) : Napi::ObjectWrap<AudioRenderer>(info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  audio_renderer_.reset(new gurum::SdlAudioRenderer);
  // audio_renderer_->EnableLog(true);
}

void AudioRenderer::Prepare(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
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
  
  assert(audio_renderer_);
  if(audio_renderer_) {
    int err; 
    err = audio_renderer_->Prepare((AVSampleFormat)sampleformat, channels, samplerate, channellayout);
    if(err) {
      LOG(ERROR) << " failed to prepare the audio renderer";
      Napi::TypeError::New(env, "prepare exception").ThrowAsJavaScriptException();
      return;
    }
  }
}


void AudioRenderer::Render(const Napi::CallbackInfo& info) {
  if(log_enabled_) LOG(INFO) << __func__;
  if (info.Length() <= 0 || !info[0].IsExternal()) {
    Napi::TypeError::New(info.Env(), "External expected").ThrowAsJavaScriptException();
    return;
  }

  Napi::External<AVFrame> external = info[0].As<Napi::External<AVFrame>>();
  AVFrame *frame = external.Data();
  assert(frame);
  audio_renderer_->Render(frame, [&](uint8_t *data, size_t len)->int {
    // Hexdump((const uint8_t *)data, (size_t)len);
    return 0;
  });
  av_frame_free(&frame);
}

void AudioRenderer::EnableLog(const Napi::CallbackInfo& info, const Napi::Value &value) {
  Napi::Env env = info.Env();
  if (info.Length() <= 0 || !value.IsBoolean()) {
    Napi::TypeError::New(env, "Boolean expected").ThrowAsJavaScriptException();
    return;
  }
  log_enabled_ = value.ToBoolean();
  if(audio_renderer_) audio_renderer_->EnableLog(log_enabled_);
}

Napi::Value AudioRenderer::log_enabled(const Napi::CallbackInfo& info) {
  return Napi::Boolean::New(info.Env(), log_enabled_);
}


void AudioRenderer::Hexdump(const uint8_t *data, size_t len) {
  for(int i=0; i < (int)len; i++) {
    fprintf(stderr, "%02x ", data[i]);

    if(((i+1)%0x10)==0) fprintf(stderr, "\n");
  }

  fprintf(stderr, "\n");

  for(int i=0; i < (int)len; i++) {
    char tmp = '.';
    if(((data[i] >= 'A') && (data[i] <= 'z')) || ((data[i] >= '0') && (data[i] <= '9'))) {
      tmp = data[i];
    }
    fprintf(stderr, "%c ", tmp);

    if(((i+1)%0x10)==0) fprintf(stderr, "\n");

    fflush(stderr);
  }
}

