#ifndef GURUM_VIDEO_RENDERER_H_
#define GURUM_VIDEO_RENDERER_H_

#include <simplemedia/config.h>
#include <SDL2/SDL.h>
extern "C" {
#include <libavutil/frame.h>
}

#include <functional>
#include "simplemedia/renderer.h"
#include "simplemedia/subtitle_renderer.h"
#include <string>

#define IMAGE_COPY	0 // TODO

namespace gurum {

class VideoRenderer : public Renderer {
public:
//  using OnInvalidated=std::function<void(void *)>;

	void SetWidth(int w) {w_=w;}
	void SetHeight(int h) {h_=h;}
	void SetPixelFormat(AVPixelFormat fmt) {pixel_fmt_=fmt;}
	virtual int Prepare()=0;
	virtual int Prepare(int w, int h, int fmt)=0;

	virtual int Resize(int width, int height)=0;

  void SetWindowTitle(const std::string &title) {
    title_ = title;
  }

//  void SetOnInvalidated(OnInvalidated on_invalidated){on_invalidated_=on_invalidated;}
//  virtual void Invalidate(/* rect */)=0;

#if 0
  virtual void *window()=0;
  virtual void *renderer()=0;
#endif

//  virtual int Blit(void *context)=0;

  void SetSubtitleRenderer(SubtitleRenderer *renderer){subtitle_renderer_=renderer;}

protected:
  AVPixelFormat pixel_fmt_=(AVPixelFormat)0;
  int w_=0;
  int h_=0;
  std::string title_;
//  OnInvalidated on_invalidated_=nullptr;
  SubtitleRenderer *subtitle_renderer_=nullptr;
};

} // namespace gurum

#endif // GURUM_VIDEO_RENDERER_H_
