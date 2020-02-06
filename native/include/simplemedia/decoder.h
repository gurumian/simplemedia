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
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
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
}

#include <assert.h>
#include <functional>
#include <atomic>

#include "pid_channel.h"
#include "timer.h"

namespace base {
class SimpleThread;
}

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

	Decoder();
	virtual ~Decoder();

	int Prepare(AVStream *stream);

	void SetPidChannel(PidChannel *pidchannel){pidchannel_=pidchannel;}

	int Start();
	int Stop();
	int Pause();

	int Flush();

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
	AVCodecContext *codec_context_=nullptr;
	AVCodec *codec_=nullptr;

	OnWillPrepare on_will_prepare_=nullptr;
	OnPrepared on_prepared_=nullptr;
	OnNullPacketSent on_null_packet_sent_=nullptr;

	PidChannel *pidchannel_=nullptr;
  std::unique_ptr<base::SimpleThread> thread_;
  std::atomic<State> state_{none};

  AVStream *stream_=nullptr;
  std::mutex lck_;
  std::condition_variable cond_;
  bool log_enabled_=false;
};

} // namespace gurum

#endif // GURUM_DECODER_H_
