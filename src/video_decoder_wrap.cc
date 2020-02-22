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

VideoDecoder::VideoDecoder(const Napi::CallbackInfo& info) : Napi::ObjectWrap<VideoDecoder>(info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  decoder_.reset(new gurum::VideoDecoder);
}


void VideoDecoder::SetPidChannel(const Napi::CallbackInfo& info, const Napi::Value &value) {
  Napi::Env env = info.Env();
  if (info.Length() <= 0 || !info[0].IsExternal()) {
    Napi::TypeError::New(env, "External expected").ThrowAsJavaScriptException();
    return;
  }

  Napi::External<gurum::PidChannel> external = value.As<Napi::External<gurum::PidChannel>>();
  gurum::PidChannel *pidchannel = external.Data();
  if(log_enabled_) LOG(INFO) << __func__ << " pidchannel: " << pidchannel;
  assert(pidchannel);
  assert(decoder_);

  decoder_->SetPidChannel(pidchannel);
}

void VideoDecoder::Prepare(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (info.Length() <= 0 || !info[0].IsExternal()) {
    Napi::TypeError::New(env, "External expected").ThrowAsJavaScriptException();
    return;
  }

  Napi::External<AVStream> external = info[0].As<Napi::External<AVStream>>();
  AVStream *strm = external.Data();
  assert(strm);
  
  if(log_enabled_) LOG(INFO) << __func__ << " strm: "<< strm;

  assert(decoder_);
  if(decoder_) {
    int err; 
    err = decoder_->Prepare(gurum::CodecParam(strm));
    if(err) {
      LOG(ERROR) << " failed to prepare the video decoder";
      Napi::TypeError::New(env, "prepare exception").ThrowAsJavaScriptException();
      return;
    }
  }
}

void VideoDecoder::Start(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  assert(decoder_);
  if(decoder_) {
    int err; 
    err = decoder_->Start();
    if(err) {
      LOG(ERROR) << " failed to start the video decoder";
      Napi::TypeError::New(env, "exception while starting a decoder").ThrowAsJavaScriptException();
      return;
    }
  }
}

void VideoDecoder::Stop(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  assert(decoder_);
  if(decoder_) {
    int err;
    err = decoder_->Stop();
    if(err) {
      LOG(ERROR) << " failed to stop the video decoder";
      Napi::TypeError::New(env, "exception while stopping a decoder").ThrowAsJavaScriptException();
      return;
    }
  }
}

void VideoDecoder::Pause(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  assert(decoder_);
  if(decoder_) {
    int err;
    err = decoder_->Pause();
    if(err) {
      LOG(ERROR) << " failed to pause the video decoder";
      Napi::TypeError::New(env, "exception while pausing a decoder").ThrowAsJavaScriptException();
      return;
    }
  }
}


Napi::Value VideoDecoder::Decode(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

  if (info.Length() <= 0 || !info[0].IsFunction()) {
    Napi::TypeError::New(env, "Function expected").ThrowAsJavaScriptException();
    return env.Undefined();
  }


  auto callback = info[0].As<Napi::Function>();
  bool sent_frame{false};
  decoder_->SetOnNullPacketSent([&](const gurum::Decoder &decoder){
    if(log_enabled_) LOG(INFO) << __func__ << " null packet!";
    callback.Call(env.Global(), {env.Null()});
    sent_frame = true;
  });

  if(sent_frame) {
    return Napi::Boolean::New(env, sent_frame);
  }

  decoder_->Decode([&](const AVFrame *arg){
    auto frame = Frame::NewInstance(env, Napi::External<AVFormatContext>::New(env, (AVFormatContext *)arg));
    callback.Call(env.Global(), {frame});
    sent_frame = true;
  });

  return Napi::Boolean::New(env, sent_frame);
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