#include "simplemedia/config.h"
#include "simplemedia/subtitle_decoder.h"
#include "log_message.h"

namespace gurum {

SubtitleDecoder::~SubtitleDecoder() {
  if(log_enabled_) LOG(INFO) << __func__;
  avsubtitle_free(&subtitle_);
}

int SubtitleDecoder::DidPrepare() {
  return 0;
}

int SubtitleDecoder::decode(AVPacket *pkt, OnSubtitleFound on_subtitle_found) {
  if(log_enabled_) LOG(INFO) << __func__;
  int decoded = 0;
  int got = 0;
  int err = 0;
  assert(codec_context_);

  err = avcodec_decode_subtitle2(codec_context_, &subtitle_, &got, pkt);
  if(err < 0) {
    LOG(ERROR) << " failed to avcodec_decode_subtitle2";
    return err;
  }

  decoded = FFMIN(err, pkt->size);
  if(decoded) {
    // TODO:
  }

  if(got && subtitle_.format ==0) {
    if(on_subtitle_found) on_subtitle_found(&subtitle_);
  }

  if(got) {
    avsubtitle_free(&subtitle_);
  }
  return 0;
}

int SubtitleDecoder::width() {
  return codec_context_->width;
}

int SubtitleDecoder::height() {
  return codec_context_->height;
}

int SubtitleDecoder::pixelFormat() {
  return (int)codec_context_->pix_fmt;
}

void SubtitleDecoder::Run() {
  while(state_==started) {
    AVPacket pkt1, *pkt = &pkt1;
    int err = pidchannel_->Pop(pkt);
    if(!err) {
      std::unique_lock<std::mutex> lk(lck_);
      decode(pkt, on_subtitle_found_);
      if(PidChannel::IsNullPacket(pkt)) {
        if(on_null_packet_sent_) on_null_packet_sent_(*this);
      }
      av_packet_unref(pkt);
    }
  }
}

} // namespace gurum