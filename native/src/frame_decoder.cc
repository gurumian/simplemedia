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
  if(err) return EAGAIN;

  if(PidChannel::IsNullPacket(pkt)) {
    Pause();
    if(on_null_packet_sent_) on_null_packet_sent_(*this);
    if(on_frame_found) on_frame_found(nullptr);
  }
  else {
    err = decode(pkt, on_frame_found);
  }

  return err;
}

int FrameDecoder::Decode(AVPacket *pkt, OnFrameFound on_frame_found) {
  return decode(pkt, on_frame_found);
}

int FrameDecoder::decode(AVPacket *pkt, OnFrameFound on_frame_found) {
  std::lock_guard<std::mutex> lk(lck_);
  int err = 0;
  err = avcodec_send_packet(codec_context_, pkt);
  if(err < 0) {
    LOG(ERROR) << __func__ << " avcodec_send_packet: " << err;
    return err;
  }

  while (!err) {
    err = avcodec_receive_frame(codec_context_, frame());
    if (err == AVERROR(EAGAIN) || err == AVERROR_EOF)
      return err;
    // if(err) continue;

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
  if(log_enabled_) LOG(INFO) << __func__ << " thread leave!";
}

} // namespace gurum
