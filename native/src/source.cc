#include <assert.h>
#include <memory>
#include <algorithm>
#include <iostream>

extern "C" {
#include <libavformat/avformat.h>
}

#include "simplemedia/source.h"
#include "simplemedia/pid_channel.h"
#include "packet_pool.h"
#include <chrono>
#include <thread>
#include "log_message.h"
#include "simple_thread.h"

namespace gurum {

Source::Source() {
  static bool inited{false};
  if(!inited) {
    avformat_network_init ();
    // LOG(INFO) << "ffmpeg configure: " << avcodec_configuration();
    // LOG(INFO) << " avcodec version: " << avcodec_version() << " ["  << LIBAVCODEC_VERSION_MAJOR << "."
    //     << LIBAVCODEC_VERSION_MINOR << "."
    //     << LIBAVCODEC_VERSION_MICRO << "]";
    inited = true;
  }

  pid_[0] = pid_[1] = pid_[2] = -1;
}

Source::~Source() {
  if(state_!=stopped) Stop();

  if(log_enabled_) LOG(INFO) << __func__;
}

void Source::SetDataSource(const std::string &dataSource) {
  std::lock_guard<std::mutex> lk(lck_);
  data_source_ = dataSource;
}

const std::string &Source::dataSource() {
  std::lock_guard<std::mutex> lk(lck_);
  return data_source_;
}

void Source::PrepareAsync(OnPrepared on_prepared) {
  if(!thread_) thread_ = base::SimpleThread::CreateThread();

  thread_->PostTask([=]{
    const AVFormatContext *fmt = Prepare();
    if(on_prepared) on_prepared(fmt);
  });
}

const AVFormatContext *Source::Prepare() {
  std::lock_guard<std::mutex> lk(lck_);
  int err{0};

  err = avformat_open_input(&fmt_, data_source_.c_str(), NULL, NULL);
  if(err < 0) {
    char buf[256];
    av_strerror(err, buf, sizeof(buf));
    LOG(ERROR) << __func__ << " " << data_source_;
    LOG(FATAL) << __func__ << " failed to avformat_open_input : " << buf;
    return nullptr;
  }

  // probe media | acquire PSI information.
  err = Scan();
  if(err < 0) {
    LOG(ERROR) <<  "Scan :" << err;
    return nullptr;
  }

  const int pool_size = 200;
  packet_pool_ = std::unique_ptr<PacketPool>(new PacketPool);
  assert(packet_pool_);
  packet_pool_->Init(pool_size);
  state_ = prepared;

  return (const AVFormatContext *)fmt_;
}

int Source::ReadAndDispatch() {
  if(log_enabled_) LOG(INFO) << __func__;
  if(state_==paused) {
    std::lock_guard<std::mutex> lk(lck_);
    av_read_play(fmt_);
    state_=started;
    return 0;
  }

  thread_->PostTask([&]{
    Run();
  });
  return 0;
}

int Source::Start() {
  if(log_enabled_) LOG(INFO) << __func__;
  nullpkt_sent_ = false;

  do {
    std::lock_guard<std::mutex> lk(lck_);
    if(!thread_) thread_= base::SimpleThread::CreateThread();
  } while(0);

  av_read_play(fmt_);
  state_ = started;

  thread_->PostTask([&]{
    while(state_ == started) {
      Run();
    }
    if(log_enabled_) LOG(INFO) << __func__ << " thread leave!";
  });
  return 0;
}

int Source::Pause() {
  std::lock_guard<std::mutex> lk(lck_);
  if(log_enabled_) LOG(INFO) << __func__;
  state_ = paused;
  av_read_pause(fmt_);
  return 0;
}

void Source::Stop() {
  if(log_enabled_) LOG(INFO) << __func__;
  if(state_==stopped) {
    LOG(WARNING) <<  " It's already stopped.";
    return;
  }

  state_ = stopped;

  std::lock_guard<std::mutex> lk(lck_);
  thread_ = nullptr;

  avformat_close_input(&fmt_);
  fmt_ = nullptr;

  if(log_enabled_) LOG(INFO) << __func__;
}

PidChannel *Source::FindPidChannelBy(uint16_t pid) {
  return pid_channel_pool_[pid];
}

PidChannel *Source::RequestPidChannel(uint16_t pid) {
  PidChannel *pid_channel = FindPidChannelBy(pid);
  if(pid_channel) {
    int ref = pid_channel->ref();
    LOG(INFO) << "ref: " << ref << " of pid: " << pid_channel->pid();
    return pid_channel;
  }

  assert(packet_pool_);
  pid_channel = new PidChannel((NativeHandle)fmt_, pid, packet_pool_.get());
  assert(pid_channel);

  if(pid_channel)
    pid_channel_pool_[pid] = pid_channel;
  return pid_channel;
}

void Source::ReleasePidChannel(PidChannel *pid_channel) {
  // TODO:  1. reference count down. 2. if 0, remove from the pool
  int ref = pid_channel->unref();
  if(ref > 0) {
    if(log_enabled_) LOG(INFO) << " reference-count : (" << ref << ") of pidchannel[" << pid_channel->pid() << "]";
    return;
  }

  for(auto pos = pid_channel_pool_.begin(); pos != pid_channel_pool_.end(); ) {
    if(pos->second == pid_channel)
      pos = pid_channel_pool_.erase(pos);
    else
      ++pos;
  }
}

int Source::Scan() {
  assert(fmt_);
  int err;
  err = avformat_find_stream_info(fmt_, NULL);
  if(err < 0) {
    LOG(ERROR) << " failed to avformat_find_stream_info() err["<< err << "]";
    return -1;
  }

  if(log_enabled_) 
    av_dump_format(fmt_, 0, data_source_.c_str(), 0);

  if(log_enabled_) LOG(INFO) << " numof streams: " << fmt_->nb_streams;

  pid_[VIDEO] = av_find_best_stream(fmt_, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
  pid_[AUDIO] = av_find_best_stream(fmt_, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
  pid_[SUBTITLE] = av_find_best_stream(fmt_, AVMEDIA_TYPE_SUBTITLE, -1, -1, NULL, 0);

  return 0;
}

void Source::Run(int unused) {
  int err{0};

  if(nullpkt_sent_) {
    LOG(WARNING) << __func__ << " null packet sent!";
    return;
  }

  AVPacket *pkt = packet_pool_->Request(500);
  if(!pkt) {
    return;
  }

  err = ReadFrame(pkt);
  if(err) {
    packet_pool_->Release(pkt);
    return;
  }

  // Video | Audio | Data.
  PidChannel *pid_channel = FindPidChannelBy(pkt->stream_index);
  if(pid_channel) {
    pid_channel->Push(pkt);
  }
  else {
    packet_pool_->Release(pkt);
  }
}

int64_t Source::GetDuration() {
  return fmt_->duration;
}


void Source::Seek(int64_t pos, int flag, OnWillSeek on_will_seek) {
  if(log_enabled_) LOG(INFO) << __func__<< " pos: [" << pos << "]";

  std::lock_guard<std::mutex> lk(lck_);
  if(on_will_seek) on_will_seek();

  for(auto pos : pid_channel_pool_) {
    if(auto pid_channel = pos.second) pid_channel->Flush();
  }

  int strm = -1;
  int64_t min_ts = INT64_MIN;
  int64_t max_ts = INT64_MAX;

  if(flag & AVSEEK_FLAG_BACKWARD) {
    max_ts=pos;
  }
  else {
    min_ts=pos;
  }

  int err = avformat_seek_file(fmt_, strm, min_ts, pos, max_ts, 0);
  if(err < 0) {
    LOG(ERROR) << " failed to avformat_seek_file(): [" << min_ts << "~" << max_ts << "]" << err;
    constexpr int64_t ms = 1000000;
    min_ts -=(5 * ms);
    err = avformat_seek_file(fmt_, strm, min_ts - (5 * ms), pos, max_ts, 0);
    if(err < 0)
      LOG(ERROR) << " failed to avformat_seek_file(): [" << min_ts << "~" << max_ts << "]" << err;
  }

  std::this_thread::yield();
}

int Source::ReadFrame(AVPacket *pkt) {
  std::unique_lock<std::mutex> lk(lck_);
  int err = av_read_frame(fmt_, pkt);
  if(err < 0) {
    if(err==AVERROR_EOF) {
      // send null packet to all pidchannels
      QueueEoS();
      lk.unlock();
      Pause();
    }
  }
  return err;
}

void Source::QueueEoS() {
  for(auto pos : pid_channel_pool_) {
    assert(pos.second);

    AVPacket *pkt = packet_pool_->Request(100);
    assert(pkt);
    if(!pkt) {
      LOG(WARNING) << __func__ << " lack of packet";
      return;
    }

    pkt->data = NULL;
    pkt->size = 0;
    pkt->stream_index = pos.first;
    pos.second->Push(pkt);
    if(log_enabled_) LOG(INFO) << " pushed a null-packet to stream[" << pos.first << "]";
  }
  nullpkt_sent_ = true;
}

AVStream *Source::FindStream(int pid) {
  if(! fmt_) return nullptr;
  return fmt_->streams[pid];
}

} // namespace gurum
