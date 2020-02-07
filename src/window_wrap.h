#ifndef GURUM_WINDOW_WRAP_H
#define GURUM_WINDOW_WRAP_H

#include <napi.h>
#include <SDL2/SDL.h>

class Window : public Napi::ObjectWrap<Window> {
 public:
  static Napi::Object Init(Napi::Env env, Napi::Object exports);
  Window(const Napi::CallbackInfo& info);
  virtual ~Window();

private:
  Napi::Value createRenderer(const Napi::CallbackInfo& info);
  void destroyRenderer(const Napi::CallbackInfo& info);

  Napi::Value pollEvent(const Napi::CallbackInfo& info);

private:
  static Napi::FunctionReference constructor;
  SDL_Window *window_{nullptr};
};

#endif // GURUM_WINDOW_WRAP_H