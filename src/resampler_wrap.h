#ifndef GURUM_RESAMPLER_WRAP_H
#define GURUM_RESAMPLER_WRAP_H

#include <napi.h>

extern "C" {
#include <libavformat/avformat.h>
}

#include "simplemedia/resampler.h"
#include <atomic>

class Resampler : public Napi::ObjectWrap<Resampler> {
 public:
  static Napi::Object Init(Napi::Env env, Napi::Object exports);
  Resampler(const Napi::CallbackInfo& info);

 private:
  static Napi::FunctionReference constructor;

  Napi::Value resample(const Napi::CallbackInfo& info);

private:
  std::unique_ptr<gurum::Resampler> resampler_{};

  bool log_enabled_{false};
  AVSampleFormat sampleformt_{};

};

#endif // GURUM_RESAMPLER_WRAP_H