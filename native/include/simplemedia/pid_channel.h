#ifndef GURUM_PIDCHANNEL_H_
#define GURUM_PIDCHANNEL_H_

#include <simplemedia/config.h>
#include <map>
#include <string>
#include <list>
#include <queue>
#include <condition_variable>
#include <mutex>
#include <memory>
extern "C" {
#include <libavformat/avformat.h>
}

typedef void *NativeHandle;

namespace gurum {

class PacketPool;
class Source;
class PidChannel {
public:
  friend class Source;

  void setNativeHandle(NativeHandle native_handle) {native_handle_ = native_handle;}
  NativeHandle nativeHandle() {return native_handle_;}

	uint16_t pid() {
		return pid_;
	}

  int Push(const AVPacket *pkt) ;
  int Pop(AVPacket *pkt);

  static bool IsNullPacket(AVPacket *pkt);

  void EnableLog(bool enable=true) {
    log_enabled_=enable;
  }

  void Flush();

private:
  explicit PidChannel(NativeHandle nativeHandle, uint16_t pid, PacketPool *packet_pool);
  virtual ~PidChannel();

private:
  int ref() {return ++ref_;}
  int unref() {return --ref_;}

private:
  NativeHandle native_handle_{nullptr};
  int ref_{1};
  uint16_t pid_{0};
  std::condition_variable cond_;
  std::mutex lck_;
  std::string pipe_{};
  int fd_[2];

  std::list<AVPacket *> que_;
  PacketPool *packet_pool_{nullptr};
  bool log_enabled_{false};
};


} // namespace gurum

#endif // GURUM_PIDCHANNEL_H_