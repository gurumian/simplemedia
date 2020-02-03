#include <simplemedia/config.h>
extern "C" {
#include <libavutil/frame.h>
}
#include <functional>

#ifndef GURUM_RENDERER_H_
#define GURUM_RENDERER_H_

namespace gurum {

class Renderer {
public:
  using OnRawData=std::function<int (uint8_t *data, size_t len)>;

  Renderer()=default;
  virtual ~Renderer()=default;
  virtual int Render(const AVFrame *frame, OnRawData on_raw_data=nullptr)=0;

};

} // namespace gurum

#endif // GURUM_RENDERER_H_
