#ifndef GURUM_SOURCE_H
#define GURUM_SOURCE_H

#include <napi.h>

extern "C" {
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
}

#include "simplemedia/source.h"
#include <atomic>

class Source : public Napi::ObjectWrap<Source> {
 public:
  static Napi::Object Init(Napi::Env env, Napi::Object exports);
  Source(const Napi::CallbackInfo& info);

 private:
  static Napi::FunctionReference constructor;

  Napi::Value dataSource(const Napi::CallbackInfo& info);
  void SetDataSource(const Napi::CallbackInfo& info, const Napi::Value &value);
  
  Napi::Value audioPid(const Napi::CallbackInfo& info);
  Napi::Value hasAudio(const Napi::CallbackInfo& info);
  Napi::Value videoPid(const Napi::CallbackInfo& info);
  Napi::Value hasVideo(const Napi::CallbackInfo& info);
  

  Napi::Value Prepare(const Napi::CallbackInfo& info);
  void Start(const Napi::CallbackInfo& info);
  void Stop(const Napi::CallbackInfo& info);
  void Pause(const Napi::CallbackInfo& info);
  void Seek(const Napi::CallbackInfo& info);

  void EnableLog(const Napi::CallbackInfo& info, const Napi::Value &value);
  Napi::Value log_enabled(const Napi::CallbackInfo& info);
  Napi::Value duration(const Napi::CallbackInfo& info);

  Napi::Value RequestPidChannel(const Napi::CallbackInfo& info);

  Napi::Value FindStream(const Napi::CallbackInfo& info);

private:
  std::unique_ptr<gurum::Source> source_{};

  bool log_enabled_{false};
};

#endif // GURUM_SOURCE_H