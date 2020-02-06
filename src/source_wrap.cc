#include "source_wrap.h"
#include <napi.h>
#include <uv.h>
#include "log_message.h"

Napi::FunctionReference Source::constructor;

Napi::Object Source::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func =
      DefineClass(env,
                  "Source",
                  {
                    InstanceMethod("prepare", &Source::Prepare),
                    InstanceMethod("start", &Source::Start),
                    InstanceMethod("stop", &Source::Stop),
                    InstanceMethod("pause", &Source::Pause),
                    InstanceMethod("requestPidChannel", &Source::RequestPidChannel),
                    InstanceMethod("findStream", &Source::FindStream),
                    InstanceAccessor("datasource", &Source::dataSource, &Source::SetDataSource),
                    InstanceAccessor("audioPid", &Source::audioPid, nullptr),
                    InstanceAccessor("hasAudio", &Source::hasAudio, nullptr),
                    InstanceAccessor("videoPid", &Source::videoPid, nullptr),
                    InstanceAccessor("hasVideo", &Source::hasVideo, nullptr),
                    InstanceAccessor("trace", &Source::log_enabled, &Source::EnableLog),
                  });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("Source", func);

  return exports;
}

Source::Source(const Napi::CallbackInfo& info) : Napi::ObjectWrap<Source>(info) {
  if(log_enabled_) LOG(INFO) << __func__;
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  source_.reset(new gurum::Source);
}

void Source::SetDataSource(const Napi::CallbackInfo& info, const Napi::Value &value) {
  Napi::Env env = info.Env();
  if (info.Length() <= 0 || !value.IsString()) {
    Napi::TypeError::New(env, "String expected").ThrowAsJavaScriptException();
    return;
  }
  source_->SetDataSource(value.ToString().Utf8Value());
}

Napi::Value Source::dataSource(const Napi::CallbackInfo& info) {
  return Napi::String::New(info.Env(), source_->dataSource());
}

Napi::Value Source::audioPid(const Napi::CallbackInfo& info) {
  return Napi::Number::New(info.Env(), source_->audioPid());
}
  
Napi::Value Source::hasAudio(const Napi::CallbackInfo& info) {
  return Napi::Boolean::New(info.Env(), source_->HasAudio());
}

Napi::Value Source::videoPid(const Napi::CallbackInfo& info) {
  return Napi::Number::New(info.Env(), source_->videoPid());
}
  
Napi::Value Source::hasVideo(const Napi::CallbackInfo& info) {
  return Napi::Boolean::New(info.Env(), source_->HasVideo());
}

Napi::Value Source::Prepare(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  auto root = Napi::Object::New(env);
  auto format = Napi::Object::New(env);

  const AVFormatContext *fmt = source_->Prepare();
  format["native"] = Napi::External<AVFormatContext>::New(env, (AVFormatContext *)fmt);
  format["duration"] = fmt->duration;
  AVDictionary* meta_data = fmt->metadata;
  AVDictionaryEntry* entry = NULL;
  while ((entry = av_dict_get((const AVDictionary*)meta_data, "", entry, AV_DICT_IGNORE_SUFFIX))) {
    format[entry->key] = entry->value;
  }

  root["format"] = format;

  auto streams = Napi::Array::New(env, fmt->nb_streams);
  for (int i = 0; i < (int)fmt->nb_streams; i++) {
    auto stream = Napi::Object::New(env);
    AVStream *strm = fmt->streams[i];
    stream["native"] = Napi::External<AVStream>::New(env, strm);
    AVCodecParameters *codec_param = strm->codecpar;
    stream["duration"] = Napi::Number::New(env, strm->duration);
    AVCodecID codec_id = codec_param->codec_id;
    stream["codec"] = Napi::String::New(env, avcodec_get_name(codec_id));
    stream["bitrate"] = Napi::Number::New(env, codec_param->bit_rate);
    stream["channels"] = Napi::Number::New(env, codec_param->channels);
    stream["samplerate"] = Napi::Number::New(env, codec_param->sample_rate);

    AVDictionary* meta_data = strm->metadata;
    while ((entry = av_dict_get((const AVDictionary*)meta_data, "", entry, AV_DICT_IGNORE_SUFFIX))) {
      stream[entry->key] = Napi::String::New(env, entry->value);
    }

    streams[i] = stream;
  }

  root["streams"] = streams;
  return root;
}

void Source::Start(const Napi::CallbackInfo& info) {
  assert(source_);
  source_->Start();
}

void Source::Stop(const Napi::CallbackInfo& info) {
  assert(source_);
  source_->Stop();
}

void Source::Pause(const Napi::CallbackInfo& info) {
  assert(source_);
  source_->Pause();
}

Napi::Value Source::RequestPidChannel(const Napi::CallbackInfo& info){
  Napi::Env env = info.Env();
  if (info.Length() <= 0) {
    Napi::TypeError::New(env, "Object expected").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  Napi::Value value;
  if (info[0].IsObject()) {
    Napi::Object obj = info[0].ToObject();
    if(!obj.HasOwnProperty("pid")) {
      Napi::TypeError::New(env, "no pid").ThrowAsJavaScriptException();
      return env.Undefined();
    }

    value = obj["pid"];
    if(! value.IsNumber()) {
      Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
      return env.Undefined();
    }
  }
  else if(info[0].IsNumber()) {
    value = info[0].ToNumber();
  }
  else {
    Napi::TypeError::New(env, "Object | Number expected").ThrowAsJavaScriptException();
    return env.Undefined();
  }

#if 1
  Napi::Number number = value.As<Napi::Number>();
  int pid = (int) number;
  gurum::PidChannel *pidchannel = source_->RequestPidChannel(pid);
  assert(pidchannel);
  return Napi::External<gurum::PidChannel>::New(env, pidchannel);
#else
  Napi::Number number = value.As<Napi::Number>();
  int pid = (int) number;
  Napi::Value pidchannel = Napi::External<gurum::PidChannel>::New(env, source_->RequestPidChannel(pid));
  Napi::Object object = PidChannel::NewInstance(info.Env(), pidchannel);
  return object;
#endif
}

Napi::Value Source::FindStream(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (info.Length() <= 0 || !info[0].IsNumber()) {
    Napi::TypeError::New(env, "Number expected").ThrowAsJavaScriptException();
    return env.Undefined();
  }

  int pid = (int) info[0].ToNumber();
  AVStream *stream = source_->FindStream(pid);
  assert(stream);
  return Napi::External<AVStream>::New(env, stream);
}

void Source::EnableLog(const Napi::CallbackInfo& info, const Napi::Value &value) {
  Napi::Env env = info.Env();
  if (info.Length() <= 0 || !value.IsBoolean()) {
    Napi::TypeError::New(env, "Boolean expected").ThrowAsJavaScriptException();
    return;
  }
  log_enabled_ = value.ToBoolean();
  if(source_) source_->EnableLog(log_enabled_);
}

Napi::Value Source::log_enabled(const Napi::CallbackInfo& info) {
  return Napi::Boolean::New(info.Env(), log_enabled_);
}
