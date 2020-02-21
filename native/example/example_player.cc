#include <simplemedia/config.h>
#include <iostream>
#include <fcntl.h>
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <map>
#include <functional>
#include <simplemedia/sdl/sdl_video_renderer.h>
#include <simplemedia/sdl/sdl_subtitle_renderer.h>
#include <simplemedia/sdl/sdl_audio_renderer.h>
#include "log_message.h"
#include <simplemedia/media_player.h>
#include <SDL2/SDL.h>


using namespace gurum;

static Uint32 MEDIA_EVENT_TYPE=0;

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

int ReadAndDispatch(SDL_Window *window, SDL_Renderer *renderer, SDL_Event *event, MediaPlayer *const player) {
  static SDL_Texture *texture=nullptr;

  SDL_WaitEvent(event);

  if(MEDIA_EVENT_TYPE==event->type) {
    Uint32 flags = SDL_GetWindowFlags(window);
    if(flags & SDL_WINDOW_FULLSCREEN_DESKTOP) {
      SDL_SetWindowFullscreen(window, flags & ~SDL_WINDOW_FULLSCREEN_DESKTOP);
    }
    return -1;
  }

  switch (event->type) {
  case SDL_KEYUP: {
    break;
  }
  case SDL_MOUSEMOTION: {
    SDL_ShowCursor(SDL_ENABLE);
    break;
  }
  case SDL_KEYDOWN:  {
    switch(event->key.keysym.sym) {
    case SDLK_LEFT: {
      int64_t pos = player->currentPosition();
      pos -= (1 * 1000000);
      player->Seek(pos);
      break;
    }
    case SDLK_RIGHT: {
      int64_t pos = player->currentPosition();
      pos += (1 * 1000000);
      if(pos < player->duration())
        player->Seek(pos);
      break;
    }
    case SDLK_UP: {
      float vol = player->volume();
      vol += 0.1f;
      player->SetVolume(vol);
      break;
    }
    case SDLK_DOWN: {
      float vol = player->volume();
      vol -= 0.1f;
      player->SetVolume(vol);
      break;
    }
    case SDLK_SPACE:{
      if(player->state() == paused) {
        player->Start();
      }
      else {
        player->Pause();
      }
      break;
    }
    case SDLK_RETURN:{
      Uint32 flags = SDL_GetWindowFlags(window);
      if(flags & SDL_WINDOW_FULLSCREEN_DESKTOP) {
        SDL_SetWindowFullscreen(window, flags & ~SDL_WINDOW_FULLSCREEN_DESKTOP);
      }
      else {
        SDL_SetWindowFullscreen(window, flags | SDL_WINDOW_FULLSCREEN_DESKTOP);
        SDL_ShowCursor(SDL_DISABLE);
      }
      break;
    }
    case SDLK_l : {
      if(texture)
        SDL_DestroyTexture(texture);
      texture=nullptr;
      break;
    }
    case SDLK_k : {
      if(player->state()!=started) {
        SdlVideoRenderer *video_renderer = dynamic_cast<SdlVideoRenderer *>(player->videoRenderer());
        if(video_renderer) {
          video_renderer->Invalidate(nullptr);
        }
      }
      break;
    }

    } // switch

    break;
  }
  case SDL_QUIT:
    return -1;
  }

  std::this_thread::yield();
  return 0;
}

SDL_Window *CreateWindow() {
  SDL_DisplayMode display_mode;
  SDL_GetDesktopDisplayMode(0, &display_mode);

  uint32_t flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
  SDL_Window* window = SDL_CreateWindow("mediaplayer - Gurum Lab.", SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED, display_mode.w, display_mode.h, flags);

  return window;
}

int main(int argc, char *argv[]) {
  int err = 0;
  std::string uri = argv[1];
  SDL_Texture *texture=nullptr;

  InitSDL();

  MEDIA_EVENT_TYPE = SDL_RegisterEvents(1);


  SDL_Window *window = CreateWindow();
  assert(window);

  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  assert(renderer);

  // 1. video
  std::unique_ptr<SdlVideoRenderer> video_renderer{ new SdlVideoRenderer(renderer) };
  std::unique_ptr<SdlSubtitleRenderer> subtitle_renderer{ new SdlSubtitleRenderer(renderer) };

  std::unique_ptr<SdlAudioRenderer> audio_renderer{ new SdlAudioRenderer };
  std::unique_ptr<MediaPlayer> player{new MediaPlayer};

  // 3. set renderers
  player->SetVideoRenderer(std::move(video_renderer));
  player->SetAudioRenderer(std::move(audio_renderer));
  player->SetSubtitleRenderer(std::move(subtitle_renderer));
  player->SetOnEndOfStream([&]{
    LOG(INFO) << __func__ << " end-of-stream!";
    // SDL_Event event;
    // SDL_memset(&event, 0, sizeof(event)); /* or SDL_zero(event) */
    // event.type = MEDIA_EVENT_TYPE;
    // event.user.code = 0;
    // event.user.data1 = 0;
    // event.user.data2 = 0;
    // SDL_PushEvent(&event);
  });

  player->SetOnStateChanged([&](gurum::State from, gurum::State to) {
    LOG(INFO) << " State changed from " << from << " to " <<  to;
  });

  // 4. set datasource
  player->EnableLog(true);
  player->SetDataSource(uri);
  player->Prepare();

  LOG(INFO) << " duration: " << player->duration();

  player->Start();

  SDL_Event event;
  while(! ReadAndDispatch(window, renderer, &event, player.get())) {
    // TODO:
  }

  player = nullptr;

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  LOG(INFO) << __func__ << " done";
  return 0;
}