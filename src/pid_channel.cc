#include <assert.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "log_message.h"
#include "simplemedia/pid_channel.h"
#include "packet_pool.h"

namespace gurum {

PidChannel::PidChannel(NativeHandle natve_handle, uint16_t pid, PacketPool *packet_pool)
: native_handle_(natve_handle), pid_(pid), packet_pool_(packet_pool) {
  assert(packet_pool_);
}

PidChannel::~PidChannel() {
	unlink(pipe_.c_str());
}

int PidChannel::Push(const AVPacket *pkt) {
	std::lock_guard<std::mutex> lk(lck_);
	que_.push_back((AVPacket *)pkt);
	return 0;
}

int PidChannel::Pop(AVPacket *pkt) {
	std::lock_guard<std::mutex> lk(lck_);

	assert(pkt);
	if(que_.empty())
		return -1;

	AVPacket *pkt1 = que_.front();
	av_packet_move_ref(pkt, pkt1);
	que_.pop_front();

	assert(packet_pool_);
	packet_pool_->Release(pkt1);

	return 0;
}

void PidChannel::Flush() {
  while(true) {
    std::unique_lock<std::mutex> lk(lck_);
    if(que_.empty())
      break;

    AVPacket *pkt1 = que_.front();
    que_.pop_front();
    lk.unlock();
    packet_pool_->Release(pkt1);
  }
}


bool PidChannel::IsNullPacket(AVPacket *pkt){
  if(pkt->size==0 || ! pkt->data) {
    return true;
  }
  return false;
}

} // namespace gurum
