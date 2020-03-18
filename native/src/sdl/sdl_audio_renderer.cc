#include <assert.h>
#include <map>
#include <math.h>
#include "simplemedia/sdl/sdl_audio_renderer.h"
#include "simplemedia/config.h"
#include "log_message.h"

namespace gurum {

static std::map<AVSampleFormat, SDL_AudioFormat> formatDictionary = {
    {AV_SAMPLE_FMT_NONE, -1},
    {AV_SAMPLE_FMT_U8, AUDIO_U8}, ///< unsigned 8 bits
    {AV_SAMPLE_FMT_S16, AUDIO_S16}, ///< signed 16 bits
    {AV_SAMPLE_FMT_S32, AUDIO_S32}, ///< signed 32 bits
    {AV_SAMPLE_FMT_FLT, AUDIO_F32}, ///< float
    {AV_SAMPLE_FMT_DBL, -1},  ///< double
    {AV_SAMPLE_FMT_U8P, -1}, ///< unsigned 8 bits, planar
    {AV_SAMPLE_FMT_S16P, AUDIO_S16LSB}, ///< signed 16 bits, planar
    {AV_SAMPLE_FMT_S32P, AUDIO_S32}, ///< signed 32 bits, planar
    {AV_SAMPLE_FMT_FLTP, AUDIO_F32}, ///< float, planar
    {AV_SAMPLE_FMT_DBLP, -1}, ///< double, planar
    {AV_SAMPLE_FMT_S64, -1}, ///< signed 64 bits, planar
    {AV_SAMPLE_FMT_S64P, -1}, ///< signed 64 bits, planar
    {AV_SAMPLE_FMT_NB, -1},
};

SdlAudioRenderer::SdlAudioRenderer() {
  static bool inited=false;
  if(! inited) {
    SDL_InitSubSystem(SDL_INIT_AUDIO);
    inited=true;
  }
}

SdlAudioRenderer::~SdlAudioRenderer() {
  if(audio_device_)
    SDL_CloseAudioDevice(audio_device_);

//  SDL_QuitSubSystem(SDL_INIT_AUDIO);
  if(log_enabled_) LOG(INFO) << __func__;
}

int SdlAudioRenderer::Prepare() {
  SDL_AudioFormat sdl_fmt = formatDictionary[(AVSampleFormat)fmt_];
  if(log_enabled_) LOG(INFO) << __func__ << " fmt : " << fmt_ << " sdl fmt: " << sdl_fmt;
  SDL_AudioSpec spec;
  SDL_memset(&spec, 0, sizeof(spec));
  SDL_memset(&obtained_, 0, sizeof(obtained_));
  spec.freq = samplerate_;
  spec.format = formatDictionary[(AVSampleFormat)fmt_];//_fmt;
  spec.channels = channels_;
  spec.silence = 0;
  #define 	SDL_AUDIO_MIN_BUFFER_SIZE   512
  #define SDL_AUDIO_MAX_CALLBACKS_PER_SEC 30
  spec.samples = FFMAX(SDL_AUDIO_MIN_BUFFER_SIZE, 2 << av_log2(spec.freq / SDL_AUDIO_MAX_CALLBACKS_PER_SEC)); // samplingFrequency * sampleBytes / 60;
  spec.callback = (SDL_AudioCallback)nullptr;
  spec.userdata = nullptr;

  audio_device_ = SDL_OpenAudioDevice(nullptr, 0, &spec, &obtained_, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_FORMAT_CHANGE);
  if(! audio_device_) {
    LOG(FATAL) << "SDL_OpenAudioDevice";
    return -1;
  }

  samplerate_ = obtained_.freq;
  SDL_PauseAudioDevice(audio_device_, 0);
  return 0;
}

int SdlAudioRenderer::Render(const AVFrame *frame, OnRawData on_raw_data) {
  std::lock_guard<std::mutex> lk(lck_);

  int err{0};

  gurum::Buffer resampled{};
  int size;
  std::tie(resampled, size) = Resample(*frame);

  std::unique_ptr<uint8_t, std::function<void(void *)>> out{
    (uint8_t *)malloc(size),
    [](void *ptr){ if(ptr) free(ptr);}
  };

  AdjustVolume(out.get(), resampled.get(), size);

  err = SDL_QueueAudio(audio_device_, (const void *)out.get(), size);
  if(err) {
    LOG(WARNING) << __func__ << "E: SDL_QueueAudio";
  }
  
  if(on_raw_data) on_raw_data(resampled.get(), size);
  return 0;
}


int SdlAudioRenderer::Prepare(AVSampleFormat fmt, int channels, int samplerate, int64_t channellayout) {
  SetSampleFormat(fmt);
  SetChannels(channels);
  SetSamplerate(samplerate);
  SetChannelLayout(channellayout);
  return Prepare();
}


void SdlAudioRenderer::SetVolume(float volume) {
  std::lock_guard<std::mutex> lk(lck_);

  if(volume < 0.0f) {
    volume =0.0f;
  }
  else if(volume > 1.0f) {
    volume=1.0f;
  }

  if(volume_==volume)
    return;

  volume_=volume;
  LOG(INFO) << __func__ << " [" << volume_ <<  "]";
}

void SdlAudioRenderer::AdjustVolume(uint8_t *out, uint8_t *data, size_t data_size) {
  SDL_memset(out, 0, data_size);
  SDL_MixAudioFormat(out, data, obtained_.format, data_size, volume_*SDL_MIX_MAXVOLUME);
}

} // namespace gurum