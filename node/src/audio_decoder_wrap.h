#ifndef GURUM_AUDIO_DECODER_WRAP_H
#define GURUM_AUDIO_DECODER_WRAP_H

#include <napi.h>

extern "C" {
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
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

  void Decode(const Napi::CallbackInfo& info);


  void SetPidChannel(const Napi::CallbackInfo& info, const Napi::Value &value);

  void SetOnFrameFound(const Napi::CallbackInfo& info, const Napi::Value &value);
  void SetOnNullPacketSent(const Napi::CallbackInfo& info, const Napi::Value &value);

  void Hexdump(const uint8_t *data, size_t len);

  Napi::Value samplerate(const Napi::CallbackInfo& info);
  Napi::Value sampleformat(const Napi::CallbackInfo& info);
  Napi::Value channels(const Napi::CallbackInfo& info);
  Napi::Value channellayout(const Napi::CallbackInfo& info);

  void EnableLog(const Napi::CallbackInfo& info, const Napi::Value &value);
  Napi::Value log_enabled(const Napi::CallbackInfo& info);

private:
  AVFrame *copyFrame(const AVFrame *frame);

private:
  static Napi::FunctionReference constructor;

  Napi::ThreadSafeFunction on_frame_found_;
  Napi::ThreadSafeFunction on_null_packet_sent_;

  std::unique_ptr<gurum::AudioDecoder> audio_decoder_{};
  bool log_enabled_{false};
};

#endif // GURUM_AUDIO_DECODER_WRAP_H


