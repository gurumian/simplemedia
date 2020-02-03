#ifndef GURUM_SDL_VIDEO_RENDERER_H_
#define GURUM_SDL_VIDEO_RENDERER_H_

#include <simplemedia/config.h>
#include <SDL2/SDL.h>
extern "C" {
#include <libavutil/frame.h>
}

#include "simplemedia/video_renderer.h"
#include <string>
#include <mutex>

namespace gurum {

class SdlVideoRenderer : public VideoRenderer {
public:
  using OnInvalidated=std::function<void(SDL_Renderer *, const SDL_Rect *rect)>;

  SdlVideoRenderer(SDL_Renderer *renderer=nullptr);
  virtual ~SdlVideoRenderer();

  void SetWidth(int w) {w_=w;}
  void SetHeight(int h) {h_=h;}
  void SetPixelFormat(AVPixelFormat fmt) {pixel_fmt_=fmt;}
  int Prepare() override;
  int Prepare(int w, int h, int fmt) override;

  int Render(const AVFrame *frame, OnRawData on_raw_data=nullptr) override;
  int Resize(int width, int height) override;

  SDL_Texture *texture() {
    return texture_;
  }

  void SetWindowTitle(const std::string &title) {
    title_ = title;
  }

  void Invalidate(const SDL_Rect *rect=nullptr);
  void SetOnInvalidated(OnInvalidated on_invalidated){on_invalidated_=on_invalidated;}

  int Blit(SDL_Texture *texture, const SDL_Rect *rect=nullptr);

private:
  int Blit(const SDL_Rect *rect=nullptr);

private:
  // SDL_Window *window_ = nullptr;
  SDL_Renderer *renderer_ = nullptr;
  SDL_Texture *texture_ = nullptr;

  // bool created_window_=false;

  AVPixelFormat pixel_fmt_=(AVPixelFormat)0;
  int w_=0;
  int h_=0;
  std::string title_;
  int64_t current_pts_=0;
  std::mutex lck_;
  OnInvalidated on_invalidated_=nullptr;
};

} // namespace gurum

#endif // GURUM_SDL_VIDEO_RENDERER_H_
