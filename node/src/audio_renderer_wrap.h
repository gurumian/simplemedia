#ifndef GURUM_AUDIO_RENDERER_WRAP_H
#define GURUM_AUDIO_RENDERER_WRAP_H

#include <napi.h>

extern "C" {
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
}

#include "simplemedia/sdl/sdl_audio_renderer.h"
#include <atomic>

class AudioRenderer : public Napi::ObjectWrap<AudioRenderer> {
 public:
  static Napi::Object Init(Napi::Env env, Napi::Object exports);
  AudioRenderer(const Napi::CallbackInfo& info);

private:
  void Prepare(const Napi::CallbackInfo& info);
  void Hexdump(const uint8_t *data, size_t len);
  void Render(const Napi::CallbackInfo& info);
  
  void EnableLog(const Napi::CallbackInfo& info, const Napi::Value &value);
  Napi::Value log_enabled(const Napi::CallbackInfo& info);

 private:
  static Napi::FunctionReference constructor;
  std::unique_ptr<gurum::SdlAudioRenderer> audio_renderer_{};

  bool log_enabled_{false};
};

#endif // GURUM_AUDIO_DECODER_WRAP_H


