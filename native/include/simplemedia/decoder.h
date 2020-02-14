#ifndef GURUM_DECODER_H_
#define GURUM_DECODER_H_

#include "simplemedia/config.h"
#include <assert.h>
#include <memory>
#include <algorithm>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stddef.h>
#include <iostream>

extern "C" {
#include <libavformat/avformat.h>
}

#include <assert.h>
#include <functional>
#include <atomic>

#include "pid_channel.h"
#include "timer.h"
#include "simple_thread.h"

namespace gurum {

class Decoder {
public:
  enum State {
    none = 0,
    prepared,
    started,
    stopped,
    paused,
  } ;


  using OnWillPrepare=std::function<int()>;
  using OnPrepared=std::function<int()>;
  using OnNullPacketSent=std::function<void(const Decoder &decoder)>;

  Decoder()=default;
  virtual ~Decoder();

  int Prepare(const AVCodecParameters *codecpar);
  int Prepare(const AVStream *strm);

  void SetPidChannel(PidChannel *pidchannel){pidchannel_=pidchannel;}

  int Start();
  int Stop();
  int Pause();

  int Flush();

  void SetTimebase(AVRational &timebase);
  const AVRational &timebase() { return timebase_; }

  void SetOnWillPrepare(OnWillPrepare on_will_prepare){on_will_prepare_=on_will_prepare;}
  void SetOnPrepared(OnPrepared on_prepared){on_prepared_=on_prepared;}

  void SetOnNullPacketSent(OnNullPacketSent on_null_packet_sent){on_null_packet_sent_=on_null_packet_sent;};

  virtual int WillPrepare(){ return 0;}
  virtual int DidPrepare(){return 0;}

  virtual AVMediaType MediaType() const=0;

  // Task for SimpleThread
  virtual void Run()=0;

  void EnableLog(bool enable=true) {log_enabled_=enable;}

protected:
  AVCodecContext *codec_context_{nullptr};
  AVCodec *codec_{nullptr};

  OnWillPrepare on_will_prepare_{nullptr};
  OnPrepared on_prepared_{nullptr};
  OnNullPacketSent on_null_packet_sent_{nullptr};

  PidChannel *pidchannel_{nullptr};
  std::unique_ptr<base::SimpleThread> thread_{};
  std::atomic<State> state_{none};

  AVRational timebase_{};
  std::mutex lck_;
  std::condition_variable cond_;
  bool log_enabled_{false};
};

} // namespace gurum

#endif // GURUM_DECODER_H_