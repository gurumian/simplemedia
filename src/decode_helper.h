#ifndef GURUM_DECODE_HELPER_H
#define GURUM_DECODE_HELPER_H

#include <napi.h>
#include <uv.h>
#include "simplemedia/frame_decoder.h"
#include "log_message.h"

namespace DecodeHelper{
  static void SetPidChannel(const Napi::CallbackInfo& info, const Napi::Value &value, gurum::FrameDecoder &decoder) {
    Napi::Env env = info.Env();
    if (info.Length() <= 0 || !info[0].IsExternal()) {
      Napi::TypeError::New(env, "External expected").ThrowAsJavaScriptException();
      return;
    }

    auto external = value.As<Napi::External<gurum::PidChannel>>();
    auto pidchannel = external.Data();
    decoder.SetPidChannel(pidchannel);
  }

  static Napi::Value Decode(const Napi::CallbackInfo& info, gurum::FrameDecoder &decoder) {
    Napi::Env env = info.Env();

    bool sent_frame{false};
    auto deferred = Napi::Promise::Deferred::New(env);

    decoder.Decode([&](const AVFrame *arg){
      sent_frame = true;
      if(arg) {
        auto frame = Frame::NewInstance(env, Napi::External<AVFrame>::New(env, (AVFrame *)arg));
        deferred.Resolve(frame);
      }
      else {
        deferred.Resolve(env.Null());
      }
    });

    if(!sent_frame) {
      deferred.Resolve(env.Undefined());
    }

    return deferred.Promise();
  }

  static void Prepare(const Napi::CallbackInfo& info, gurum::FrameDecoder &decoder) {
    Napi::Env env = info.Env();
    if (info.Length() <= 0 || !info[0].IsExternal()) {
      Napi::TypeError::New(env, "External expected").ThrowAsJavaScriptException();
      return;
    }

    auto external = info[0].As<Napi::External<AVStream>>();
    auto strm = external.Data();
    assert(strm);

    int err{0};
    err = decoder.Prepare(gurum::CodecParam(strm));
    if(err) {
      LOG(ERROR) << " failed to prepare the video decoder";
      Napi::TypeError::New(env, "prepare exception").ThrowAsJavaScriptException();
      return;
    }
  }

  static void Start(const Napi::CallbackInfo& info, gurum::FrameDecoder &decoder) {
    Napi::Env env = info.Env();
    int err{0};
    err = decoder.Start();
    if(err) {
      LOG(ERROR) << " failed to start the audio decoder";
      Napi::TypeError::New(env, "exception while starting a decoder").ThrowAsJavaScriptException();
      return;
    }
  }

  static void Stop(const Napi::CallbackInfo& info, gurum::FrameDecoder &decoder) {
    Napi::Env env = info.Env();
    int err{0};
    err = decoder.Stop();
    if(err) {
      LOG(ERROR) << " failed to stop the audio decoder";
      Napi::TypeError::New(env, "exception while stopping a decoder").ThrowAsJavaScriptException();
      return;
    }
  }

  static void Pause(const Napi::CallbackInfo& info, gurum::FrameDecoder &decoder) {
    Napi::Env env = info.Env();
    int err{0};
    err = decoder.Pause();
    if(err) {
      LOG(ERROR) << " failed to pause the audio decoder";
      Napi::TypeError::New(env, "pause exception").ThrowAsJavaScriptException();
      return;
    }
  }

  static void EnableLog(const Napi::CallbackInfo& info, const Napi::Value &value, gurum::FrameDecoder &decoder) {
    Napi::Env env = info.Env();
    if (info.Length() <= 0 || !value.IsBoolean()) {
      Napi::TypeError::New(env, "Boolean expected").ThrowAsJavaScriptException();
      return;
    }
    bool log_enabled = value.ToBoolean();
    decoder.EnableLog(log_enabled);
  }
}

#endif // GURUM_DECODE_HELPER_H