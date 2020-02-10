#ifndef GURUM_VIDEO_RENDERER_H_
#define GURUM_VIDEO_RENDERER_H_

#include <simplemedia/config.h>
// extern "C" {
// #include <libavutil/frame.h>
// }

#include <functional>
#include "simplemedia/renderer.h"
#include "simplemedia/subtitle_renderer.h"
#include <string>

#define IMAGE_COPY	0 // TODO

namespace gurum {

class VideoRenderer : public Renderer {
public:
	void SetWidth(int w) {w_=w;}
	void SetHeight(int h) {h_=h;}
	void SetPixelFormat(AVPixelFormat fmt) {pixel_fmt_=fmt;}
	virtual int Prepare()=0;
	virtual int Prepare(int w, int h, int fmt)=0;

	virtual int Resize(int width, int height)=0;

  void SetWindowTitle(const std::string &title) {
    title_ = title;
  }

  void SetSubtitleRenderer(SubtitleRenderer *renderer){subtitle_renderer_=renderer;}

  void EnableLog(bool enable=true) {
    log_enabled_=enable;
  }

protected:
  AVPixelFormat pixel_fmt_{(AVPixelFormat)0};
  int w_{0};
  int h_{0};
  std::string title_{};
  SubtitleRenderer *subtitle_renderer_{nullptr};

  bool log_enabled_{false};
};

} // namespace gurum

#endif // GURUM_VIDEO_RENDERER_H_