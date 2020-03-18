#ifndef GURUM_AUDIO_RENDERER_H_
#define GURUM_AUDIO_RENDERER_H_

#include "simplemedia/config.h"

extern "C" {
#include <libavutil/frame.h>
#include <libavutil/avstring.h>
#include <libavutil/eval.h>
#include <libavutil/mathematics.h>
#include <libavutil/pixdesc.h>
#include <libavutil/imgutils.h>
#include <libavutil/dict.h>
#include <libavutil/parseutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/avassert.h>
#include <libavutil/time.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavcodec/avfft.h>
#if defined(USE_SWRESAMPLE)
#include <libswresample/swresample.h>
#endif
#if CONFIG_AVFILTER
# include <libavfilter/avfilter.h>
# include <libavfilter/buffersink.h>
# include <libavfilter/buffersrc.h>

#include <libavutil/frame.h>
#endif
}

#include <string>
#include <mutex>
#include <condition_variable>
#include <tuple>
#include <functional>
#include "simplemedia/renderer.h"
#include "simplemedia/types.h"

namespace gurum {

class AudioRenderer : public Renderer{
public:
  AudioRenderer(){}
  virtual ~AudioRenderer(){}

  virtual int Prepare()=0;
  virtual int Prepare(AVSampleFormat fmt, int channels, int samplerate, int64_t channellayout)=0;

  virtual void SetSampleFormat(AVSampleFormat fmt){ fmt_=fmt;}
  virtual void SetChannels(int channels){ channels_=channels;}
  virtual void SetSamplerate(int samplerate){ samplerate_=samplerate;}
  virtual void SetChannelLayout(int64_t channel_layout){channel_layout_=channel_layout;}

  void EnableLog(bool enable=true) {
    log_enabled_=enable;
  }

  virtual void SetVolume(float volume) { volume_=volume;}
  float volume() { return volume_;}

  std::tuple<gurum::Buffer, int> Resample(const AVFrame &frame);


protected:
  AVSampleFormat fmt_{AV_SAMPLE_FMT_NONE};
  int channels_{0};
  int samplerate_{0};
  int64_t channel_layout_{0};

  std::mutex lck_;
  std::condition_variable cond_;
#if defined(USE_SWRESAMPLE)
  struct SwrContext *swr_{nullptr};
#endif
  bool log_enabled_{false};

  float volume_{0.5f};
};

} // namespace gurum

#endif // GURUM_AUDIO_RENDERER_H_