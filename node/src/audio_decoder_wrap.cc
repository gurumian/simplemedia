#include "audio_decoder_wrap.h"
#include <napi.h>
#include <uv.h>
#include "log_message.h"
#include <future>


struct Buffer {
  uint8_t *data;
  size_t size;
};

Napi::FunctionReference AudioDecoder::constructor;

Napi::Object AudioDecoder::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func =
      DefineClass(env,
                  "AudioDecoder",
                  {
                    InstanceMethod("prepare", &AudioDecoder::Prepare),
                    InstanceMethod("start", &AudioDecoder::Start),
                    InstanceMethod("stop", &AudioDecoder::Stop),
                    InstanceMethod("pause", &AudioDecoder::Pause),
                    InstanceMethod("decode", &AudioDecoder::Decode),
                    InstanceAccessor("pidchannel", nullptr, &AudioDecoder::SetPidChannel),
                    InstanceAccessor("sampleformat", &AudioDecoder::sampleformat, nullptr),
                    InstanceAccessor("samplerate", &AudioDecoder::samplerate, nullptr),
                    InstanceAccessor("channels", &AudioDecoder::channels, nullptr),
                    InstanceAccessor("channellayout", &AudioDecoder::channellayout, nullptr),
                    InstanceAccessor("trace", &AudioDecoder::log_enabled, &AudioDecoder::EnableLog),
                  });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("AudioDecoder", func);

  return exports;
}

AudioDecoder::AudioDecoder(const Napi::CallbackInfo& info) : Napi::ObjectWrap<AudioDecoder>(info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  audio_decoder_.reset(new gurum::AudioDecoder);
}


void AudioDecoder::SetPidChannel(const Napi::CallbackInfo& info, const Napi::Value &value) {
  if(log_enabled_) LOG(INFO) << __func__;
  Napi::Env env = info.Env();
  if (info.Length() <= 0 || !info[0].IsExternal()) {
    Napi::TypeError::New(env, "Object expected").ThrowAsJavaScriptException();
    return;
  }

  Napi::External<gurum::PidChannel> external = value.As<Napi::External<gurum::PidChannel>>();
  gurum::PidChannel *pidchannel = external.Data();
  if(log_enabled_) LOG(INFO) << __func__ << " pidchannel: " << pidchannel;
  assert(pidchannel);
  assert(audio_decoder_);

  audio_decoder_->SetPidChannel(pidchannel);
}

void AudioDecoder::Prepare(const Napi::CallbackInfo& info) {
  if(log_enabled_) LOG(INFO) << __func__;
  Napi::Env env = info.Env();
  if (info.Length() <= 0 || !info[0].IsExternal()) {
    Napi::TypeError::New(env, "Object expected").ThrowAsJavaScriptException();
    return;
  }

  Napi::External<AVStream> external = info[0].As<Napi::External<AVStream>>();
  AVStream *strm = external.Data();
  assert(strm);
  
  if(log_enabled_) LOG(INFO) << __func__ << " strm: "<< strm;

  assert(audio_decoder_);
  if(audio_decoder_) {
    int err; 
    err = audio_decoder_->Prepare(strm);
    if(err) {
      LOG(ERROR) << " failed to prepare the audio decoder";
      Napi::TypeError::New(env, "prepare exception").ThrowAsJavaScriptException();
      return;
    }
  }
}

void AudioDecoder::Start(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  assert(audio_decoder_);
  if(audio_decoder_) {
    int err; 
    err = audio_decoder_->Start();
    if(err) {
      LOG(ERROR) << " failed to start the audio decoder";
      Napi::TypeError::New(env, "start exception").ThrowAsJavaScriptException();
      return;
    }
  }
}

void AudioDecoder::Stop(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  assert(audio_decoder_);
  if(audio_decoder_) {
    int err;
    err = audio_decoder_->Stop();
    if(err) {
      LOG(ERROR) << " failed to start the audio decoder";
      Napi::TypeError::New(env, "start exception").ThrowAsJavaScriptException();
      return;
    }
  }
}

void AudioDecoder::Pause(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  assert(audio_decoder_);
  if(audio_decoder_) {
    int err;
    err = audio_decoder_->Pause();
    if(err) {
      LOG(ERROR) << " failed to start the audio decoder";
      Napi::TypeError::New(env, "start exception").ThrowAsJavaScriptException();
      return;
    }
  }
}


void AudioDecoder::Decode(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() <= 0 || !info[0].IsFunction()) {
    Napi::TypeError::New(env, "Function expected").ThrowAsJavaScriptException();
    return;
  }

  assert(audio_decoder_);

  auto callback = info[0].As<Napi::Function>();
  bool eos = false;
  audio_decoder_->SetOnNullPacketSent([&](const gurum::Decoder &decoder){
    if(log_enabled_) LOG(INFO) << __func__ << " null packet!";
    callback.Call(env.Global(), {env.Null()});
    eos = true;
  });

  if(eos) {
    return;
  }

  audio_decoder_->Decode([&](const AVFrame *frame){
    callback.Call(env.Global(), {Napi::External<AVFrame>::New(env, copyFrame(frame))});   
  });
}

AVFrame *AudioDecoder::copyFrame(const AVFrame *frame) {
  AVFrame *copied = av_frame_alloc();
  copied->format = frame->format;
  copied->width = frame->width;
  copied->height = frame->height;
  copied->channels = frame->channels;
  copied->channel_layout = frame->channel_layout;
  copied->nb_samples = frame->nb_samples;
  av_frame_get_buffer(copied, 32);
  av_frame_copy(copied, frame);
  av_frame_copy_props(copied, frame);
  return copied;
}

Napi::Value AudioDecoder::samplerate(const Napi::CallbackInfo& info) {
  return Napi::Number::New(info.Env(), audio_decoder_->samplerate());
}

Napi::Value AudioDecoder::sampleformat(const Napi::CallbackInfo& info) {
  return Napi::Number::New(info.Env(), audio_decoder_->sampleFormat());
}

Napi::Value AudioDecoder::channels(const Napi::CallbackInfo& info) {
  return Napi::Number::New(info.Env(), audio_decoder_->channels());
}

Napi::Value AudioDecoder::channellayout(const Napi::CallbackInfo& info) {
  return Napi::Number::New(info.Env(), audio_decoder_->channellayout());
}

void AudioDecoder::EnableLog(const Napi::CallbackInfo& info, const Napi::Value &value) {
  Napi::Env env = info.Env();
  if (info.Length() <= 0 || !value.IsBoolean()) {
    Napi::TypeError::New(env, "String expected").ThrowAsJavaScriptException();
    return;
  }
  log_enabled_ = value.ToBoolean();
  if(audio_decoder_) audio_decoder_->EnableLog(log_enabled_);
}

Napi::Value AudioDecoder::log_enabled(const Napi::CallbackInfo& info) {
  return Napi::Boolean::New(info.Env(), log_enabled_);
}

void AudioDecoder::Hexdump(const uint8_t *data, size_t len) {
  for(int i=0; i < (int)len; i++) {
    fprintf(stderr, "%02x ", data[i]);

    if(((i+1)%0x10)==0) fprintf(stderr, "\n");
  }

  fprintf(stderr, "\n");

  for(int i=0; i < (int)len; i++) {
    char tmp = '.';
    if(((data[i] >= 'A') && (data[i] <= 'z')) || ((data[i] >= '0') && (data[i] <= '9'))) {
      tmp = data[i];
    }
    fprintf(stderr, "%c ", tmp);

    if(((i+1)%0x10)==0) fprintf(stderr, "\n");

    fflush(stderr);
  }
}

