#include <assert.h>

#include "simplemedia/sdl/sdl_subtitle_renderer.h"
#include <unistd.h>
#include <iostream>
#include "log_message.h"

namespace gurum {

SdlSubtitleRenderer::SdlSubtitleRenderer(SDL_Renderer *renderer)
: renderer_(renderer){}

SdlSubtitleRenderer::~SdlSubtitleRenderer() {

  if(sub_convert_ctx_) {
    sws_freeContext(sub_convert_ctx_);
    sub_convert_ctx_=nullptr;
  }
}

int SdlSubtitleRenderer::Prepare() {
  Resize(w_, h_);
  return 0;
}

int SdlSubtitleRenderer::Prepare(int width, int height, int fmt) {
  LOG(INFO) << __func__ << " w: " << width << ", h: " << height << " fmt: " << fmt;
  SetWidth(width);
  SetHeight(height);
  SetPixelFormat((AVPixelFormat)fmt);
  Prepare();
  return 0;
}

int SdlSubtitleRenderer::Render(const AVSubtitle *subtitle, OnRawData on_raw_data) {
  uint8_t* pixels[4];
  int pitch[4];

  rect_.x=w_;
  rect_.y=h_;
  rect_.w=rect_.h=0;

  for (int i = 0; i < (int)subtitle->num_rects; i++) {
    AVSubtitleRect *sub_rect = subtitle->rects[i];
    sub_rect->x = av_clip(sub_rect->x, 0, w_ );
    sub_rect->y = av_clip(sub_rect->y, 0, h_);
    sub_rect->w = av_clip(sub_rect->w, 0, w_ - sub_rect->x);
    sub_rect->h = av_clip(sub_rect->h, 0, h_ - sub_rect->y);

    UpdateRect(*sub_rect);

    sub_convert_ctx_ = sws_getCachedContext(sub_convert_ctx_,
                               sub_rect->w, sub_rect->h, AV_PIX_FMT_PAL8,
                               sub_rect->w, sub_rect->h, AV_PIX_FMT_BGRA,
                               0, NULL, NULL, NULL);
    if (!sub_convert_ctx_) {
        LOG(ERROR) << " failed to texture_sws_getCachedContext()";
        return -1;
    }

    if (!SDL_LockTexture(texture_, (SDL_Rect *)sub_rect, (void **)pixels, pitch)) {
      sws_scale(sub_convert_ctx_, (const uint8_t * const *)sub_rect->data, sub_rect->linesize,
                0, sub_rect->h, pixels, pitch);
      SDL_UnlockTexture(texture_);
    }

    if(on_raw_data) on_raw_data(sub_rect->data[i], sub_rect->linesize[i]);
  }

  start_display_time_ = subtitle->pts + subtitle->start_display_time * INT64_C(1000);
  end_display_time_ = subtitle->pts + subtitle->end_display_time * INT64_C(1000);
  LOG(INFO) << " " << start_display_time_ << " ~ " << end_display_time_ << " pts: " << subtitle->pts;

  return 0;
}

int SdlSubtitleRenderer::Resize(int width, int height) {
  LOG(INFO) << __func__;
  int access;
  int w, h;
  int err;

  Uint32 fmt = SDL_PIXELFORMAT_ARGB8888; //SDL_PIXELFORMAT_YV12; // TODO:
  err = SDL_QueryTexture(texture_, &fmt, &access, &w, &h);
  if(err < 0 || width != w || height != h) {
    void *pixels;
    int pitch;
    SDL_DestroyTexture(texture_);
    if (!(texture_ = SDL_CreateTexture(renderer_, fmt, SDL_TEXTUREACCESS_STREAMING, width, height))) {
      LOG(ERROR) << " failed to SDL_CreateTexture";
      return -1;
    }

    if (SDL_SetTextureBlendMode(texture_, SDL_BLENDMODE_BLEND) < 0) {
      LOG(ERROR) << " failed to SDL_SetTextureBlendMode";
      return -1;
    }

    if (SDL_LockTexture(texture_, NULL, &pixels, &pitch) < 0) {
      LOG(ERROR) << " failed to SDL_LockTexture";
      return -1;
    }

    memset(pixels, 0, pitch * height);
    SDL_UnlockTexture(texture_);
  }
  return err;
}

void SdlSubtitleRenderer::Blit(void *renderer_arg, int64_t pts) {
  if(pts > end_display_time_)
    return;


  if(pts >= start_display_time_) {
    SDL_Renderer *renderer=(SDL_Renderer *)renderer_arg;
    SDL_RenderCopy(renderer, texture_, &rect_, &rect_);
//    LOG(INFO) << " [" << start_display_time_ << " ~ " << end_display_time_ << " pts: " <<pts;
  }
}

void SdlSubtitleRenderer::EraseTexture() {
  uint8_t *pixels;
  int pitch;
  if (!SDL_LockTexture(texture_, (SDL_Rect *) NULL, (void **)&pixels, &pitch)) {
    memset(pixels, 0, pitch * w_);
    SDL_UnlockTexture(texture_);
  }
}

void SdlSubtitleRenderer::UpdateRect(const AVSubtitleRect &rect) {

  int top[2], bottom[2], left[2], right[2];

  left[0] = rect.x;
  right[0] = rect.x + rect.w;
  top[0] = rect.y;
  bottom[0] = rect.y + rect.h;

  left[1] = rect_.x;
  right[1] = rect_.x + rect_.w;
  top[1] = rect_.y;
  bottom[1] = rect_.y + rect_.h;

  if(left[0] < left[1])
    left[1] = left[0];

  if(right[0] > right[1])
    right[1] = right[0] ;

  if(top[0] < top[1])
    top[1] = top[0];

  if(bottom[0] > bottom[1])
    bottom[1]= bottom[0];

  rect_.x = left[1];
  rect_.y = top[1];
  rect_.w = right[1] - left[1];
  rect_.h = bottom[1] - top[1];
  LOG(INFO) << " (" << rect_.x << "," << rect_.y << "," << rect_.w << "," << rect_.h << ")";
}

} // namespace gurum