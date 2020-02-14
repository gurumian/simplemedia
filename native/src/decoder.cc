#include <string>
#include "simplemedia/decoder.h"
#include "log_message.h"
#include "simple_thread.h"

namespace gurum {

Decoder::~Decoder() {
  if(codec_context_) {
    avcodec_close(codec_context_);
    avcodec_free_context(&codec_context_);
    codec_context_=nullptr;
  }
}

int Decoder::Prepare(const AVStream *strm) {
  int err = Prepare(strm->codecpar);
  if(err) {
    return err;
  }

  timebase_ = strm->time_base;
  return 0;
}

int Decoder::Prepare(const AVCodecParameters *codecpar) {
  std::lock_guard<std::mutex> lk(lck_);

  int err;
  if(on_will_prepare_) {
    err = on_will_prepare_();
    LOG(WARNING) << " WillPrepare return -1";
    return -1;
  }

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
  avcodec_open2(codec_context_, codec_, NULL);

  err = DidPrepare();

  state_=prepared;

  if(on_prepared_) {
    err = on_prepared_();
    assert(err==0);
  }

  return 0;
}

int Decoder::Start() {
  assert(pidchannel_);
  state_=started;
  if(! thread_)
    thread_= base::SimpleThread::CreateThread();


  using namespace std::placeholders;
  thread_->PostTask(std::bind(&Decoder::Run, this));

  cond_.notify_one();
  return 0;
}

int Decoder::Pause() {
  state_=paused;
  return 0;
}

int Decoder::Stop() {
  state_=stopped;

  cond_.notify_all();

  if(thread_) {
    thread_=nullptr;
  }

  LOG(INFO) << __func__;
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

void Decoder::SetTimebase(AVRational &timebase) {
  timebase_ = timebase;
}

} // namespace gurum