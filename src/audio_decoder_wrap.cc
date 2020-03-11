#include "audio_decoder_wrap.h"
#include <napi.h>
#include <uv.h>
#include "log_message.h"
#include <future>
#include "frame_wrap.h"

Napi::FunctionReference AudioDecoder::constructor;

Napi::Object AudioDecoder::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  constexpr auto name = "AudioDecoder";

  Napi::Function func =
      DefineClass(env,
                  name,
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

  exports.Set(name, func);

  return exports;
}

AudioDecoder::AudioDecoder(const Napi::CallbackInfo& info) : Decoder<AudioDecoder>(info) {
  std::unique_ptr<gurum::AudioDecoder> decoder{new gurum::AudioDecoder};
  decoder_ = std::move(decoder);
}

Napi::Value AudioDecoder::samplerate(const Napi::CallbackInfo& info) {
  auto decoder = static_cast<gurum::AudioDecoder *>(decoder_.get());
  return Napi::Number::New(info.Env(), decoder->samplerate());
}

Napi::Value AudioDecoder::sampleformat(const Napi::CallbackInfo& info) {
  auto decoder = static_cast<gurum::AudioDecoder *>(decoder_.get());
  return Napi::Number::New(info.Env(), decoder->sampleFormat());
}

Napi::Value AudioDecoder::channels(const Napi::CallbackInfo& info) {
  auto decoder = static_cast<gurum::AudioDecoder *>(decoder_.get());
  return Napi::Number::New(info.Env(), decoder->channels());
}

Napi::Value AudioDecoder::channellayout(const Napi::CallbackInfo& info) {
  auto decoder = static_cast<gurum::AudioDecoder *>(decoder_.get());
  return Napi::Number::New(info.Env(), decoder->channellayout());
}