#include "video_renderer_wrap.h"
#include <napi.h>
#include <uv.h>
#include "log_message.h"
#include <unistd.h>
#include <SDL2/SDL.h>

Napi::FunctionReference VideoRenderer::constructor;

Napi::Object VideoRenderer::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func =
      DefineClass(env,
                  "VideoRenderer",
                  {
                    InstanceMethod("prepare", &VideoRenderer::Prepare),
                    InstanceMethod("render", &VideoRenderer::Render),
                    InstanceAccessor("trace", &VideoRenderer::log_enabled, &VideoRenderer::EnableLog),
                  });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("VideoRenderer", func);

  return exports;
}

VideoRenderer::VideoRenderer(const Napi::CallbackInfo& info) : Napi::ObjectWrap<VideoRenderer>(info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  if (info.Length() <= 0 || !info[0].IsExternal()) {
    Napi::TypeError::New(env, "External expected").ThrowAsJavaScriptException();
    return;
  }

  auto external = info[0].As<Napi::External<SDL_Renderer>>();
  renderer_.reset(new gurum::SdlVideoRenderer(external.Data()));
}

void VideoRenderer::Prepare(const Napi::CallbackInfo& info) {
  int pixelformat, width, height;
  Napi::Env env = info.Env();
  if (info.Length() <= 0 || !info[0].IsObject()) {
    Napi::TypeError::New(env, "Object expected").ThrowAsJavaScriptException();
    return;
  }

  Napi::Object obj = info[0].ToObject();
  if(!obj.HasOwnProperty("pixelformat")) {
    Napi::TypeError::New(env, "no pixelformat").ThrowAsJavaScriptException();
    return;
  }

  Napi::Value value = obj["pixelformat"];
  if(! value.IsNumber()) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
    return;
  }

  pixelformat = (int)value.ToNumber();




  if(!obj.HasOwnProperty("width")) {
    Napi::TypeError::New(env, "no width").ThrowAsJavaScriptException();
    return;
  }

  value = obj["width"];
  if(! value.IsNumber()) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
    return;
  }

  width = (int) value.ToNumber();


  if(!obj.HasOwnProperty("height")) {
    Napi::TypeError::New(env, "no height").ThrowAsJavaScriptException();
    return;
  }
  
  value = obj["height"];
  if(! value.IsNumber()) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
    return;
  }

  height = (int) value.ToNumber();

  assert(renderer_);
  if(renderer_) {
    int err; 
    err = renderer_->Prepare(width, height, pixelformat);
    if(err) {
      LOG(ERROR) << " failed to prepare the video renderer";
      Napi::TypeError::New(env, "prepare exception").ThrowAsJavaScriptException();
      return;
    }
  }
}


void VideoRenderer::Render(const Napi::CallbackInfo& info) {
  if (info.Length() <= 0 || !info[0].IsExternal()) {
    Napi::TypeError::New(info.Env(), "External expected").ThrowAsJavaScriptException();
    return;
  }

  Napi::External<AVFrame> external = info[0].As<Napi::External<AVFrame>>();
  AVFrame *frame = external.Data();
  assert(frame);
  renderer_->Render(frame, [&](uint8_t *data, size_t len)->int {
    return 0;
  });
  av_frame_free(&frame);
}

void VideoRenderer::Resize(const Napi::CallbackInfo& info) {
  if (info.Length() <= 0 || !info[0].IsNumber()) {
    Napi::TypeError::New(info.Env(), "Number expected").ThrowAsJavaScriptException();
    return;
  }

  if (!info[1].IsNumber()) {
    Napi::TypeError::New(info.Env(), "Number expected").ThrowAsJavaScriptException();
    return;
  }


  int width = (int) info[0].ToNumber();
  int height = (int) info[1].ToNumber();
  renderer_->Resize(width, height);
}

void VideoRenderer::EnableLog(const Napi::CallbackInfo& info, const Napi::Value &value) {
  Napi::Env env = info.Env();
  if (info.Length() <= 0 || !value.IsBoolean()) {
    Napi::TypeError::New(env, "Boolean expected").ThrowAsJavaScriptException();
    return;
  }
  log_enabled_ = value.ToBoolean();
  // if(renderer_) renderer_->EnableLog(log_enabled_);
}

Napi::Value VideoRenderer::log_enabled(const Napi::CallbackInfo& info) {
  return Napi::Boolean::New(info.Env(), log_enabled_);
}