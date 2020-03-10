#ifndef GURUM_AUDIO_DECODER_WRAP_H
#define GURUM_AUDIO_DECODER_WRAP_H

#include <napi.h>

extern "C" {
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavformat/avformat.h>
}

#include "decoder_wrap.h"
#include "simplemedia/audio_decoder.h"


class AudioDecoder : public Decoder<AudioDecoder> {
 public:
  static Napi::Object Init(Napi::Env env, Napi::Object exports);
  AudioDecoder(const Napi::CallbackInfo& info);

private:
  Napi::Value samplerate(const Napi::CallbackInfo& info);
  Napi::Value sampleformat(const Napi::CallbackInfo& info);
  Napi::Value channels(const Napi::CallbackInfo& info);
  Napi::Value channellayout(const Napi::CallbackInfo& info);

private:
  static Napi::FunctionReference constructor;
};

#endif // GURUM_AUDIO_DECODER_WRAP_H