#ifndef GURUM_VIDEO_RENDERER_WRAP_H
#define GURUM_VIDEO_RENDERER_WRAP_H

#include <napi.h>

extern "C" {
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavformat/avformat.h>
}

#include "simplemedia/sdl/sdl_video_renderer.h"
#include <atomic>

class VideoRenderer : public Napi::ObjectWrap<VideoRenderer> {
 public:
  static Napi::Object Init(Napi::Env env, Napi::Object exports);
  VideoRenderer(const Napi::CallbackInfo& info);

private:
  void Prepare(const Napi::CallbackInfo& info);
  void Render(const Napi::CallbackInfo& info);
  void Resize(const Napi::CallbackInfo& info);
  
  void EnableLog(const Napi::CallbackInfo& info, const Napi::Value &value);
  Napi::Value log_enabled(const Napi::CallbackInfo& info);

 private:
  static Napi::FunctionReference constructor;
  std::unique_ptr<gurum::SdlVideoRenderer> renderer_{};

  bool log_enabled_{false};
};

#endif // GURUM_VIDEO_RENDERER_WRAP_H