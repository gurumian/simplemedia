#ifndef GURUM_DECODE_HELPER_H
#define GURUM_DECODE_HELPER_H

#include <napi.h>
#include <uv.h>
#include "simplemedia/frame_decoder.h"
#include "log_message.h"

namespace DecodeHelper{
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
}

#endif // GURUM_DECODE_HELPER_H