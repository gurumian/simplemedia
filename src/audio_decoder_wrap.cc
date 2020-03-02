#include "audio_decoder_wrap.h"
#include <napi.h>
#include <uv.h>
#include "log_message.h"
#include <future>
#include "frame_wrap.h"
#include "decode_helper.h"

Napi::FunctionReference AudioDecoder::constructor;

Napi::Object AudioDecoder::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func =
      DefineClass(env,
                  "AudioDecoder",
                  {
                    InstanceMethod("prepare", &AudioDecoder::Prepare),
                    InstanceMethod("start", &AudioDecoder::Start),
                    InstanceMethod("stop", &AudioDecoder::Stop),
                    InstanceMethod("pause", &AudioDecoder::Pause),
                    InstanceMethod("decode", &AudioDecoder::Decode),
                    InstanceMethod("flush", &AudioDecoder::Flush),
                    InstanceAccessor("pidchannel", nullptr, &AudioDecoder::SetPidChannel),
                    InstanceAccessor("sampleformat", &AudioDecoder::sampleformat, nullptr),
                    InstanceAccessor("samplerate", &AudioDecoder::samplerate, nullptr),
                    InstanceAccessor("channels", &AudioDecoder::channels, nullptr),
                    InstanceAccessor("channellayout", &AudioDecoder::channellayout, nullptr),
                    InstanceAccessor("trace", &AudioDecoder::log_enabled, &AudioDecoder::EnableLog),
                  });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("AudioDecoder", func);

  return exports;
}

AudioDecoder::AudioDecoder(const Napi::CallbackInfo& info) : Napi::ObjectWrap<AudioDecoder>(info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  decoder_.reset(new gurum::AudioDecoder);
}


void AudioDecoder::SetPidChannel(const Napi::CallbackInfo& info, const Napi::Value &value) {
  DecodeHelper::SetPidChannel(info, value, *decoder_);
}

void AudioDecoder::Prepare(const Napi::CallbackInfo& info) {
  DecodeHelper::Prepare(info, *decoder_);
}

void AudioDecoder::Start(const Napi::CallbackInfo& info) {
  DecodeHelper::Start(info, *decoder_);
}

void AudioDecoder::Stop(const Napi::CallbackInfo& info) {
  DecodeHelper::Stop(info, *decoder_);
}

void AudioDecoder::Pause(const Napi::CallbackInfo& info) {
  DecodeHelper::Pause(info, *decoder_);
}


Napi::Value AudioDecoder::Decode(const Napi::CallbackInfo& info) {
  return DecodeHelper::Decode(info, *decoder_);
}

void AudioDecoder::Flush(const Napi::CallbackInfo& info) {
  if(decoder_) decoder_->Flush();
}

Napi::Value AudioDecoder::samplerate(const Napi::CallbackInfo& info) {
  return Napi::Number::New(info.Env(), decoder_->samplerate());
}

Napi::Value AudioDecoder::sampleformat(const Napi::CallbackInfo& info) {
  return Napi::Number::New(info.Env(), decoder_->sampleFormat());
}

Napi::Value AudioDecoder::channels(const Napi::CallbackInfo& info) {
  return Napi::Number::New(info.Env(), decoder_->channels());
}

Napi::Value AudioDecoder::channellayout(const Napi::CallbackInfo& info) {
  return Napi::Number::New(info.Env(), decoder_->channellayout());
}

void AudioDecoder::EnableLog(const Napi::CallbackInfo& info, const Napi::Value &value) {
  DecodeHelper::EnableLog(info, value, *decoder_);
}

Napi::Value AudioDecoder::log_enabled(const Napi::CallbackInfo& info) {
  return Napi::Boolean::New(info.Env(), log_enabled_);
}