#ifndef GURUM_SUBTITLE_RENDERER_H_
#define GURUM_SUBTITLE_RENDERER_H_

#include <simplemedia/config.h>
extern "C" {
#include <libavutil/frame.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

#include <functional>
#include <string>

namespace gurum {

class SubtitleRenderer  {
public:
  using OnRawData=std::function<int (uint8_t *data, size_t len)>;

  SubtitleRenderer()=default;
	virtual ~SubtitleRenderer()=default;

  virtual int Render(const AVSubtitle *subtitle, OnRawData on_raw_data=nullptr)=0;

	void SetWidth(int w) {w_=w;}
	void SetHeight(int h) {h_=h;}
	void SetPixelFormat(AVPixelFormat fmt) {pixel_fmt_=fmt;}
	virtual int Prepare()=0;
	virtual int Prepare(int w, int h, int fmt)=0;

	virtual int Resize(int width, int height)=0;
  virtual void Blit(void *renderer, int64_t pts)=0;

protected:
  AVPixelFormat pixel_fmt_=(AVPixelFormat)0;
  int w_=0;
  int h_=0;
  int64_t start_display_time_=0;
  int64_t end_display_time_=0;
};

} // namespace gurum

#endif // GURUM_SUBTITLE_RENDERER_H_