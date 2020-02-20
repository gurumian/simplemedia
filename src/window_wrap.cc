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
SDL_Window *CreateWindow(const std::string &title, int width, int height) {
  SDL_DisplayMode display_mode;

  if(!width || !height) {
    SDL_GetDesktopDisplayMode(0, &display_mode);
    width = display_mode.w;
    height = display_mode.h;
  }

  uint32_t flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
  SDL_Window* window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED, width, height, flags);

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
                    InstanceAccessor("fullscreen", &Window::fullscreen, &Window::SetFullscreen),
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

  std::string title{};
  int width = 0;
  int height = 0;

  if(info.Length() > 0) {
    if(!info[0].IsObject()) {
      Napi::TypeError::New(env, "Object expected").ThrowAsJavaScriptException();
      return;
    }

    Napi::Object obj = info[0].ToObject();
    if(obj.HasOwnProperty("title")) {
      Napi::Value value =  obj["title"];
      if(! value.IsString()) {
        Napi::TypeError::New(env, "String expected").ThrowAsJavaScriptException();
        return;
      }
      title = value.ToString();
    }

    if(obj.HasOwnProperty("width")) {
      Napi::Value value =  obj["width"];
      if(! value.IsNumber()) {
        Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
        return;
      }
      width = (int)value.ToNumber();
    }

    if(obj.HasOwnProperty("height")) {
      Napi::Value value =  obj["height"];
      if(! value.IsNumber()) {
        Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
        return;
      }
      height = (int)value.ToNumber();
    }
  }

  window_ = CreateWindow(title, width, height);
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
    auto obj = Napi::Object::New(env);
    obj["type"] = Napi::Number::New(env, event.type);
    switch(event.type) {
    case SDL_KEYUP:
    case SDL_KEYDOWN: {
      obj["key"] = Napi::Number::New(env, event.key.keysym.sym);
      break;
    }
    }

    return obj;
  }
  return env.Null();
}

void Window::SetFullscreen(const Napi::CallbackInfo& info, const Napi::Value &value) {
  Napi::Env env = info.Env();
  if (info.Length() <= 0 || !value.IsBoolean()) {
    Napi::TypeError::New(env, "Boolean expected").ThrowAsJavaScriptException();
    return;
  }

  Uint32 flags = SDL_GetWindowFlags(window_);
  if(! value.ToBoolean()) {
    SDL_SetWindowFullscreen(window_, flags & ~SDL_WINDOW_FULLSCREEN_DESKTOP);
  }
  else {
    SDL_SetWindowFullscreen(window_, flags | SDL_WINDOW_FULLSCREEN_DESKTOP);
    SDL_ShowCursor(SDL_DISABLE);
  }
}

Napi::Value Window::fullscreen(const Napi::CallbackInfo& info) {
  Uint32 flags = SDL_GetWindowFlags(window_);
  return Napi::Boolean::New(info.Env(), flags & SDL_WINDOW_FULLSCREEN_DESKTOP);
}