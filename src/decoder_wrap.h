#ifndef GURUM_DECODER_WRAP_H
#define GURUM_DECODER_WRAP_H

#include <napi.h>
#include "simplemedia/frame_decoder.h"
#include "log_message.h"
#include "frame_wrap.h"
#include <vector>


template <class T>
class Decoder : public Napi::ObjectWrap<T> {
public:
  Decoder(const Napi::CallbackInfo& info) : Napi::ObjectWrap<T>(info){
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
  }

protected:
  void Prepare(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() <= 0 || !info[0].IsExternal()) {
      Napi::TypeError::New(env, "External expected").ThrowAsJavaScriptException();
      return;
    }

    auto external = info[0].As<Napi::External<AVStream>>();
    auto strm = external.Data();
    assert(strm);

    int err{0};
    err = decoder_->Prepare(gurum::CodecParam(strm));
    if(err) {
      LOG(ERROR) << " failed to prepare the video decoder";
      Napi::TypeError::New(env, "prepare exception").ThrowAsJavaScriptException();
      return;
    }
  }

  void Start(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    int err{0};
    err = decoder_->Start();
    if(err) {
      LOG(ERROR) << " failed to start the audio decoder";
      Napi::TypeError::New(env, "exception while starting a decoder").ThrowAsJavaScriptException();
      return;
    }
  }

  void Stop(const Napi::CallbackInfo& info){
    Napi::Env env = info.Env();
    int err{0};
    err = decoder_->Stop();
    if(err) {
      LOG(ERROR) << " failed to stop the audio decoder";
      Napi::TypeError::New(env, "exception while stopping a decoder").ThrowAsJavaScriptException();
      return;
    }
  }  
  
  void Pause(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    int err{0};
    err = decoder_->Pause();
    if(err) {
      LOG(ERROR) << " failed to pause the audio decoder";
      Napi::TypeError::New(env, "pause exception").ThrowAsJavaScriptException();
      return;
    }
  }

  Napi::Value Decode(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    auto deferred = Napi::Promise::Deferred::New(env);

    std::vector<Napi::Object> frames;
    int err{0};
    do {
      err = decoder_->Decode([&](const AVFrame *arg){
        if(arg) {
          auto frame = Frame::NewInstance(env, Napi::External<AVFrame>::New(env, (AVFrame *)arg));
          frames.push_back(frame);
        }
      });

      if(err == AVERROR_EOF) {
        deferred.Resolve(env.Null());
        return deferred.Promise();
      }
    }
    while(0 /*(err == EAGAIN) || (frames.empty())*/); // lack of packet

    auto data = Napi::Array::New(env, frames.size());
    if(frames.empty()) {
      deferred.Resolve(data);
    }
    else {
      const auto count = frames.size();
      for(int i = 0; i < count; i++) {
        data[i] = frames[i];
      }
      deferred.Resolve(data);
    }
    return deferred.Promise();
  }

  void Flush(const Napi::CallbackInfo& info) {
    if(decoder_) decoder_->Flush();
  }

  void SetPidChannel(const Napi::CallbackInfo& info, const Napi::Value &value) {
    Napi::Env env = info.Env();
    if (info.Length() <= 0 || !info[0].IsExternal()) {
      Napi::TypeError::New(env, "External expected").ThrowAsJavaScriptException();
      return;
    }

    auto external = value.As<Napi::External<gurum::PidChannel>>();
    auto pidchannel = external.Data();
    decoder_->SetPidChannel(pidchannel);
  }

  void EnableLog(const Napi::CallbackInfo& info, const Napi::Value &value) {
    Napi::Env env = info.Env();
    if (info.Length() <= 0 || !value.IsBoolean()) {
      Napi::TypeError::New(env, "Boolean expected").ThrowAsJavaScriptException();
      return;
    }
    bool log_enabled = value.ToBoolean();
    decoder_->EnableLog(log_enabled);
  }

  Napi::Value log_enabled(const Napi::CallbackInfo& info) {
    return Napi::Boolean::New(info.Env(), log_enabled_);
  }

protected:
  static Napi::FunctionReference constructor;
  std::unique_ptr<gurum::FrameDecoder> decoder_{};
  bool log_enabled_{false};
};

#endif // GURUM_DECODER_WRAP_H