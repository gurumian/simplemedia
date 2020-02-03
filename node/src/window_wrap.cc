#include <napi.h>
#include <uv.h>
#include "window_wrap.h"
#include "log_message.h"

Napi::FunctionReference Window::constructor;


static
int InitSDL() {
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  static const int kStencilBits = 8;  // Skia needs 8 stencil bits
  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, kStencilBits);

  SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

  int flags = SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS;
  SDL_Init(flags);

  return 0;
}

static
SDL_Window *CreateWindow() {
  SDL_DisplayMode display_mode;
  SDL_GetDesktopDisplayMode(0, &display_mode);

  uint32_t flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
  SDL_Window* window = SDL_CreateWindow("mediaplayer - Gurum Lab.", SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED, display_mode.w, display_mode.h, flags);

  return window;
}

Napi::Object Window::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func =
      DefineClass(env,
                  "Window",
                  {
                    InstanceMethod("createRenderer", &Window::createRenderer),
                    InstanceMethod("destroyRenderer", &Window::destroyRenderer),
                    InstanceMethod("pollEvent", &Window::pollEvent),
                  });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("Window", func);


  InitSDL();

  return exports;
}

Window::Window(const Napi::CallbackInfo& info) : Napi::ObjectWrap<Window>(info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  window_ = CreateWindow();
}

Window::~Window() {
  SDL_DestroyWindow(window_);
  SDL_Quit();
}

Napi::Value Window::createRenderer(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  SDL_Renderer *renderer = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
  return Napi::External<SDL_Renderer>::New(env, renderer);
}

void Window::destroyRenderer(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() <= 0 || !info[0].IsExternal()) {
    Napi::TypeError::New(env, "External expected").ThrowAsJavaScriptException();
    return;
  }

  auto external = info[0].As<Napi::External<SDL_Renderer>>();
  SDL_DestroyRenderer(external.Data());
}

Napi::Value Window::pollEvent(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  SDL_Event event;
  if(SDL_PollEvent(&event)) {
    return Napi::Number::New(info.Env(), event.key.keysym.sym);
  }
  return env.Null();
}
