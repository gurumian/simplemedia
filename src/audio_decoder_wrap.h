#ifndef GURUM_AUDIO_DECODER_WRAP_H
#define GURUM_AUDIO_DECODER_WRAP_H

#include <napi.h>

extern "C" {
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavformat/avformat.h>
}

#include "simplemedia/audio_decoder.h"


class AudioDecoder : public Napi::ObjectWrap<AudioDecoder> {
 public:
  static Napi::Object Init(Napi::Env env, Napi::Object exports);
  AudioDecoder(const Napi::CallbackInfo& info);

private:
  void Prepare(const Napi::CallbackInfo& info);
  void Start(const Napi::CallbackInfo& info);
  void Stop(const Napi::CallbackInfo& info);
  void Pause(const Napi::CallbackInfo& info);

  Napi::Value Decode(const Napi::CallbackInfo& info);
  void Flush(const Napi::CallbackInfo& info);


  void SetPidChannel(const Napi::CallbackInfo& info, const Napi::Value &value);

  Napi::Value samplerate(const Napi::CallbackInfo& info);
  Napi::Value sampleformat(const Napi::CallbackInfo& info);
  Napi::Value channels(const Napi::CallbackInfo& info);
  Napi::Value channellayout(const Napi::CallbackInfo& info);

  void EnableLog(const Napi::CallbackInfo& info, const Napi::Value &value);
  Napi::Value log_enabled(const Napi::CallbackInfo& info);

private:
  static Napi::FunctionReference constructor;
  std::unique_ptr<gurum::AudioDecoder> decoder_{};
  bool log_enabled_{false};
};

#endif // GURUM_AUDIO_DECODER_WRAP_H