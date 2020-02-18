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

AVFrame *FrameDecoder::frame() {
  if(! frame_) {
    frame_ = av_frame_alloc();
  }
  return frame_;
}

int FrameDecoder::DidPrepare() {
  return 0;
}

int FrameDecoder::Decode(OnFrameFound on_frame_found) {
  AVPacket pkt1, *pkt = &pkt1;
  int err = pidchannel_->Pop(pkt);
  if(!err) {
    std::unique_lock<std::mutex> lk(lck_);
    decode(pkt, on_frame_found);
    if(PidChannel::IsNullPacket(pkt)) {
      if(on_null_packet_sent_) on_null_packet_sent_(*this);
    }
    av_packet_unref(pkt);
  }
  return err;
}

int FrameDecoder::decode(AVPacket *pkt, OnFrameFound on_frame_found) {
  int err = 0;
  assert(codec_context_);

  err = avcodec_send_packet(codec_context_, pkt);
  if(err < 0) {
    LOG(ERROR) << __func__ << " E: avcodec_send_packet: " << err;
    return err;
  }

  while (!err) {
    err = avcodec_receive_frame(codec_context_, frame());
    if(err) continue;

    if(timebase().num == 0 && timebase().den == 0) {
      frame_->pts = frame_->best_effort_timestamp;
    }
    else {
      const AVRational microseconds = {1, 1000000};
      frame_->pts = av_rescale_q(frame_->pkt_dts, timebase(), microseconds);
    }

    if(on_frame_found) on_frame_found(frame_);
  }
  return 0;
}

void FrameDecoder::Run() {
  while(state_==started) {
    Decode(on_frame_found_);
  }
}

} // namespace gurum