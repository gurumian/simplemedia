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
  spec.samples = 1024; // samplingFrequency * sampleBytes / 60;
  spec.callback = (SDL_AudioCallback)nullptr;
  spec.userdata = nullptr;

  audio_device_ = SDL_OpenAudioDevice(nullptr, 0, &spec, &obtained_, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
  if(! audio_device_) {
    LOG(FATAL) << "SDL_OpenAudioDevice";
    return -1;
  }

  samplerate_ = obtained_.freq;

#if defined(USE_SWRESAMPLE)
  swr_ = swr_alloc();
  CHECK_NOTNULL(swr_);

  av_opt_set_int(swr_, "in_channel_count", channels_, 0);
  av_opt_set_int(swr_, "out_channel_count", channels_, 0);
  av_opt_set_int(swr_, "in_channel_layout", channel_layout_, 0);
  av_opt_set_int(swr_, "out_channel_layout", channel_layout_, 0);
  av_opt_set_int(swr_, "in_sample_rate", samplerate_, 0);
  av_opt_set_int(swr_, "out_sample_rate", samplerate_, 0);
  av_opt_set_sample_fmt(swr_, "in_sample_fmt", (AVSampleFormat)fmt_, 0);
  av_opt_set_sample_fmt(swr_, "out_sample_fmt", AV_SAMPLE_FMT_S16,  0);
  err = swr_init(swr_);
  if(err) {
    LOG(FATAL) << " failed to swr_init";
  }
#endif

  SDL_PauseAudioDevice(audio_device_, 0);
  return 0;
}

int SdlAudioRenderer::Render(const AVFrame *frame, OnRawData on_raw_data) {
  std::lock_guard<std::mutex> lk(lck_);

  int err;

#if defined(USE_SWRESAMPLE)
  uint8_t *data = nullptr;
  err = av_samples_alloc(&data, NULL, channels_, frame->nb_samples, AV_SAMPLE_FMT_S16, 0);
  if( !data || err < 0) {
    LOG(ERROR) << "failed to av_samples_alloc()";
    return -1;
  }

  int resample_count = swr_convert(swr_, &data, frame->nb_samples, (const uint8_t **) frame->extended_data, frame->nb_samples);
  if(resample_count <= 0) {
    LOG(ERROR) << "failed to convert" << resample_count;
    return -1;
  }

  int size = av_samples_get_buffer_size(NULL, channels_, resample_count, AV_SAMPLE_FMT_S16, 0);

  err = SDL_QueueAudio(audio_device_, (const void *)data, size);
  assert(err==0);

  if(on_raw_data) on_raw_data(data, size);

  av_freep(&data);
#else
  int data_size = av_get_bytes_per_sample((AVSampleFormat)frame->format);

  for (int i=0; i<frame->nb_samples; i++) {
    for (int ch=0; ch<channels_; ch++) {
      uint8_t buf[4];
      AdjustVolume(buf, (frame->data[ch] + data_size*i), data_size);
      err = SDL_QueueAudio(audio_device_, (const void *)buf, data_size);
      if(err) {
        LOG(WARNING) << " failed to SDL_QueueAudio():" << err;
      }
      if(on_raw_data) on_raw_data((uint8_t *)buf, data_size);
    }
  }
 #endif
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

  int sign;
  if(volume_ > volume) {
    sign = -1;
  }
  else {
    sign = 1;
  }

  volume_=volume;
  LOG(INFO) << __func__ << " [" << volume_ <<  "]";
}

void SdlAudioRenderer::AdjustVolume(uint8_t *out, uint8_t *data, size_t data_size) {
  SDL_memset(out, 0, data_size);
  SDL_MixAudioFormat(out, data, obtained_.format, data_size, volume_*SDL_MIX_MAXVOLUME);
}

} // namespace gurum
