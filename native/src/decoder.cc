#include <string>
#include "simplemedia/decoder.h"
#include "log_message.h"
#include "simple_thread.h"

namespace gurum {

Decoder::~Decoder() {
  if(state_!=stopped) {
    Stop();
  }

  if(codec_context_) {
    avcodec_close(codec_context_);
    avcodec_free_context(&codec_context_);
    codec_context_=nullptr;
  }
}

int Decoder::Prepare(const CodecParam &param) {
  return Prepare(param.codecpar, param.timebase);
}

int Decoder::Prepare(const AVCodecParameters *codecpar, const AVRational &timebase) {
  std::lock_guard<std::mutex> lk(lck_);

  int err = 0;
  codec_ = avcodec_find_decoder(codecpar->codec_id);
  if(! codec_) {
    LOG(ERROR) << " failed to avcodec_find_decoder";
    return -1;
  }

  codec_context_ = avcodec_alloc_context3(codec_);
  if(! codec_context_) {
    LOG(ERROR) << " failed to avcodec_alloc_context3";
    return -1;
  }

  avcodec_parameters_to_context(codec_context_, codecpar);

  AVDictionary *opts = nullptr;
  err = avcodec_open2(codec_context_, codec_, &opts);
  if(err < 0) {
    LOG(ERROR) << __func__ << " failed to avcodec_open2()";
    return err;
  }

  timebase_ = timebase;
  state_ = prepared;
  return 0;
}

int Decoder::Prepare(const AVStream *strm, OnWillPrepare on_will_prepared, OnPrepared on_prepared) {
  int err = 0;
  if(on_will_prepared) {
    err = on_will_prepared();
    LOG(WARNING) << " WillPrepare return -1";
    return -1;
  }

  err = Prepare(strm->codecpar, strm->time_base);
  if(err) {
    LOG(ERROR) << __func__ << " failed to prepare: " << err;
    return -1;
  }

  if(on_prepared) {
    err = on_prepared();
    assert(! err);
  }
  return err;
}

int Decoder::Start() {
  assert(pidchannel_);
  state_=started;
  if(!thread_) thread_= base::SimpleThread::CreateThread();

  thread_->PostTask(std::bind(&Decoder::Run, this));
  return 0;
}

int Decoder::Pause() {
  if(log_enabled_) LOG(INFO) << __func__;
  state_ = paused;
  return 0;
}

int Decoder::Stop() {
  if(state_==stopped) {
    return 0;
  }

  state_ = stopped;

  if(thread_) thread_ = nullptr;

  if(log_enabled_) LOG(INFO) << __func__;
  return 0;
}

int Decoder::Flush() {
  if(pidchannel_)
    pidchannel_->Flush();

  std::lock_guard<std::mutex> lk(lck_);

  if(codec_context_)
    avcodec_flush_buffers(codec_context_);
  return 0;
}

void Decoder::SetTimebase(const AVRational &timebase) {
  timebase_ = timebase;
}

} // namespace gurum