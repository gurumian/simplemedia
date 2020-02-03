#include <assert.h>

#include "simplemedia/sdl/sdl_video_renderer.h"
extern "C" {
#include <libavutil/imgutils.h>
}

#include <unistd.h>
#include <iostream>
#include "log_message.h"

namespace gurum {

SdlVideoRenderer::SdlVideoRenderer(SDL_Renderer *renderer)
: renderer_(renderer){
}

SdlVideoRenderer::~SdlVideoRenderer() {
  if(texture_) {
    SDL_DestroyTexture(texture_);
    texture_ = nullptr;
  }
}

int SdlVideoRenderer::Prepare() {
  Resize(w_, h_);

  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
  SDL_RenderSetLogicalSize(renderer_, w_, h_);
  SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
  SDL_RenderClear(renderer_);
  SDL_RenderPresent(renderer_);

  return 0;
}

int SdlVideoRenderer::Prepare(int width, int height, int fmt) {
  SetWidth(width);
  SetHeight(height);
  SetPixelFormat((AVPixelFormat)fmt);
  Prepare();
  return 0;
}

int SdlVideoRenderer::Render(const AVFrame *frame, OnRawData on_raw_data) {
  int err;
  err = SDL_UpdateYUVTexture(texture_, nullptr,
      frame->data[0], frame->linesize[0],
      frame->data[1], frame->linesize[1],
      frame->data[2], frame->linesize[2]);

  current_pts_ = frame->pts;

  Invalidate();

  if(on_raw_data) on_raw_data(frame->data[0], frame->linesize[0]); //

  return 0;
}

int SdlVideoRenderer::Resize(int width, int height) {
  int access;
  int w, h;
  int err;
  Uint32 fmt = SDL_PIXELFORMAT_YV12; // TODO:
  err = SDL_QueryTexture(texture_, &fmt, &access, &w, &h);
  if(err < 0 || width != w || height != h) {
    void *pixels;
    int pitch;
    SDL_DestroyTexture(texture_);
    if (!(texture_ = SDL_CreateTexture(renderer_, fmt, SDL_TEXTUREACCESS_STREAMING, width, height)))
      return -1;

    if (SDL_SetTextureBlendMode(texture_, SDL_BLENDMODE_BLEND) < 0)
      return -1;

    if (SDL_LockTexture(texture_, NULL, &pixels, &pitch) < 0)
      return -1;
    memset(pixels, 0, pitch * height);
    SDL_UnlockTexture(texture_);
  }
  return err;
}

// draw-direct. consider renaming.
int SdlVideoRenderer::Blit(SDL_Texture *texture, const SDL_Rect *rect) {
  std::lock_guard<std::mutex> lk(lck_);

  Blit(rect);
  SDL_RenderCopy(renderer_, texture, rect, rect);
  SDL_RenderPresent(renderer_);
  return 0;
}

int SdlVideoRenderer::Blit(const SDL_Rect *rect) {
  SDL_RenderCopy(renderer_, texture_, rect, rect);

  if(subtitle_renderer_)
    subtitle_renderer_->Blit(renderer_, current_pts_);

  return 0;
}

void SdlVideoRenderer::Invalidate(const SDL_Rect *rect) {
  std::lock_guard<std::mutex> lk(lck_);
  SDL_RenderClear(renderer_);
  Blit();
  if(on_invalidated_) on_invalidated_(renderer_, rect);

  SDL_RenderPresent(renderer_);
}

} // namespace gurum
