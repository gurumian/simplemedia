#ifndef GURUM_SDL_AUDIO_RENDERER_H_
#define GURUM_SDL_AUDIO_RENDERER_H_

#include "simplemedia/config.h"

#include <SDL2/SDL.h>
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

#include <SDL2/SDL.h>
#include "simplemedia/audio_renderer.h"

namespace gurum {

class SdlAudioRenderer : public AudioRenderer{
public:
  SdlAudioRenderer();
  virtual ~SdlAudioRenderer();

  int Prepare() override;
  int Prepare(AVSampleFormat fmt, int channels, int samplerate, int64_t channellayout) override;

  // @override
  int Render(const AVFrame *frame, OnRawData on_raw_data=nullptr) override;

  void EnableLog(bool enable) {
    log_enabled_=enable;
  }

  void SetVolume(float volume) override;

private:
  void AdjustVolume(uint8_t *out, uint8_t *data, size_t data_size);

private:
  SDL_AudioDeviceID audio_device_=0;
  SDL_AudioSpec obtained_;
};

} // namespace gurum

#endif // GURUM_SDL_AUDIO_RENDERER_H_