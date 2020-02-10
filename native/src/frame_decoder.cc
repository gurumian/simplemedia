#include "simplemedia/config.h"
#include "simplemedia/frame_decoder.h"
#include "log_message.h"

namespace gurum {

FrameDecoder::~FrameDecoder() {
  if(frame_) {
    av_frame_free(&frame_);
    frame_=nullptr;
  }
}

int FrameDecoder::DidPrepare() {
  frame_ = av_frame_alloc();
  assert(frame_);

  AVDictionary *opts = nullptr;
//	av_opt_set_int(&opts, "refcounted_frames",  1, 0);

  assert(codec_context_);
  assert(codec_);
  int err = avcodec_open2(codec_context_, codec_, &opts);
  assert(err==0);

	return 0;
}

int FrameDecoder::Decode(OnFrameFound on_frame_found) {
  AVPacket pkt1, *pkt = &pkt1;
  int err = pidchannel_->Pop(pkt);
  if(!err) {
    decode(pkt, on_frame_found);
    if(PidChannel::IsNullPacket(pkt)) {
      if(on_null_packet_sent_) on_null_packet_sent_(*this);
    }
    av_packet_unref(pkt);
  }
  return err;
}

void FrameDecoder::Run() {
  while(state_!=stopped) {
    std::unique_lock<std::mutex> lk(lck_);
    if(state_==paused) {
      cond_.wait(lk);
      continue;
    }

    Decode(on_frame_found_);
  }
}

} // namespace gurum