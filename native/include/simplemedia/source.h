#ifndef GURUM_SOURCE_H_
#define GURUM_SOURCE_H_

#include <simplemedia/config.h>
#include <thread>
#include <map>
#include <list>
#include <mutex>
#include <condition_variable>
#include <memory>
#include "pid_channel.h"

extern "C" {
#include <libavformat/avformat.h>
}
#include <functional>
#include <atomic>
#include "simple_thread.h"

namespace gurum {
  enum State {
    none = 0,
    prepared,
    started,
    stopped,
    paused,
  } ;

  enum {
    VIDEO,
    AUDIO,
    SUBTITLE,
    NUMOF_STREAM_TYPE
  };

using PidChannelDictionary=std::map<uint16_t, PidChannel *>;

class PacketPool;
class Source {
  using OnWillSeek=std::function<void()>;
  using OnWillPrepare=std::function<int()>;
  using OnPrepared=std::function<int(const AVFormatContext *fmt)>;

public:
  Source();
  virtual ~Source();

  void SetDataSource(const std::string &data_source);
  const std::string &dataSource();

  virtual void ParseDataSource() {};

  const AVFormatContext *Prepare();
  void PrepareAsync(OnPrepared on_prepared);

  int Start();
  void Stop();
  int Pause();

  int ReadAndDispatch();

  gurum::State state() {return state_;}
  void SetState(gurum::State state);

  void EnableLog(bool enable=true) {log_enabled_=enable;}

  PidChannel *RequestPidChannel(uint16_t pid);
  void ReleasePidChannel(PidChannel *handle);

  PidChannel *FindPidChannelBy(uint16_t pid);

  bool HasVideo();
  bool HasAudio();
  bool HasSubtitle();

  int videoPid();
  int audioPid();
  int subtitlePid();

  int64_t GetDuration();
  void Seek(int64_t pos, int flag, OnWillSeek on_will_seek=nullptr);

  AVStream *FindStream(int pid);

protected:
  virtual void Run(int unused);

private:
  int Scan();
  int ReadFrame(AVPacket *pkt);

protected:
  std::string data_source_{};
  std::unique_ptr<base::SimpleThread> thread_{};

  bool log_enabled_{false};
  bool thread_started_{false};

  PidChannelDictionary pid_channel_pool_;

  std::atomic<gurum::State> state_{none};
  AVFormatContext *fmt_{nullptr};

  std::condition_variable cond_;
  std::mutex lck_;
  bool nullpkt_sent_{false};
  std::unique_ptr<PacketPool> packet_pool_;

  int pid_[NUMOF_STREAM_TYPE];
};

inline bool Source::HasVideo() {
  return (pid_[VIDEO] >= 0) ? true : false;
}

inline bool Source::HasAudio() {
  return (pid_[AUDIO] >= 0) ? true : false;
}

inline bool Source::HasSubtitle() {
  return (pid_[SUBTITLE] >= 0) ? true : false;
}

inline int Source::videoPid() {
  return pid_[VIDEO];
}

inline int Source::audioPid() {
  return pid_[AUDIO];
}

inline int Source::subtitlePid() {
  return pid_[SUBTITLE];
}


} // namespace gurum

#endif // GURUM_SOURCE_H_