#include "packet_pool.h"
#include <chrono>
#include "log_message.h"

namespace gurum {

int PacketPool::Init(int num) {
  for(int i = 0; i < num; i++) {
    AVPacket *pkt = createPacket();
    lst_.push_back(pkt);
  }
  return 0;
}

AVPacket *PacketPool::createPacket() {
  AVPacket *pkt = av_packet_alloc();
  assert(pkt);
  av_init_packet(pkt);
  pkt->data = nullptr;
  return pkt;
}

AVPacket *PacketPool::Request(int timeout) {
  std::unique_lock<std::mutex> lk(lck_);

  AVPacket *pkt = nullptr;

  for(;;) {
    pkt = lst_.front();
    if(pkt) {
      lst_.pop_front();
      av_packet_unref(pkt);
      break;
    }
    else {
      if(timeout==0) {
        cond_.wait(lk);
      }
      else {
        std::cv_status st = cond_.wait_for(lk, timeout*std::chrono::milliseconds(1));
        if(st == std::cv_status::timeout) {
          return nullptr;
        }
      }
    }
  }
  return pkt;
}

void PacketPool::Release(AVPacket *pkt, bool notify) {
  std::lock_guard<std::mutex> lk(lck_);
	av_packet_unref(pkt);
  lst_.push_back(pkt);
  if(notify)
    cond_.notify_one();
}

} // namespace gurum