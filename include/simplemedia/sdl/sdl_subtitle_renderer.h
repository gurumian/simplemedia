#ifndef GURUM_SDL_SUBTITLE_RENDERER_H_
#define GURUM_SDL_SUBTITLE_RENDERER_H_

#include <simplemedia/config.h>
#include <SDL2/SDL.h>
extern "C" {
#include <libavutil/frame.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

#include "simplemedia/subtitle_renderer.h"
#include <string>

namespace gurum {

class SdlSubtitleRenderer : public SubtitleRenderer {
public:
  SdlSubtitleRenderer(SDL_Renderer *renderer=nullptr);
  virtual ~SdlSubtitleRenderer();

  void SetWidth(int w) {w_=w;}
  void SetHeight(int h) {h_=h;}
  void SetPixelFormat(AVPixelFormat fmt) {pixel_fmt_=fmt;}
  int Prepare() override;
  int Prepare(int w, int h, int fmt) override;

  int Render(const AVSubtitle *subtitle, OnRawData on_raw_data=nullptr) override;
  int Resize(int width, int height) override;
 
  void EraseTexture();

  SDL_Renderer *renderer() {
    return (SDL_Renderer *) renderer_;
  }

  SDL_Texture *texture() {
    return texture_;
  }

  void Blit(void *renderer, int64_t pts) override;

private:
  void UpdateRect(const AVSubtitleRect &rect);

private:
  SDL_Renderer *renderer_ = nullptr;
  SDL_Texture *texture_ = nullptr;

  AVPixelFormat pixel_fmt_=(AVPixelFormat)0;
  int w_=0;
  int h_=0;
  std::string title_;
  struct SwsContext *sub_convert_ctx_=nullptr;
  SDL_Rect rect_;
};

} // namespace gurum

#endif // GURUM_SDL_SUBTITLE_RENDERER_H_
