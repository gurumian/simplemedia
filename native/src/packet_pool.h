#ifndef GURUM_PACKETPOOL_H_
#define GURUM_PACKETPOOL_H_

#include <list>
#include <mutex>
#include <condition_variable>

extern "C" {
#include <libavformat/avformat.h>
}

namespace gurum {

class PacketPool {
public:
  PacketPool()=default;
  ~PacketPool()=default;

  int Prepare(int num);
  AVPacket *Request(int timeout=0);
  void Release(AVPacket *pkt, bool notify=true);

private:
  std::mutex lck_;
  std::condition_variable cond_;
  std::list<AVPacket *> lst_;
};

} // namespace gurum

#endif // GURUM_PACKETPOOL_H_