#include "encoder.h"
#include "log_message.h"

namespace gurum {

Encoder::Encoder() {
}

Encoder::~Encoder() {
  if(codec_context_) {
    avcodec_free_context(&codec_context_);
    codec_context_ = nullptr;
  }
}


int Encoder::Prepare(const CodecParam &param) {
  int err{0};
  codec_ = avcodec_find_encoder(param.codecpar->codec_id);
  assert(codec_);

  codec_context_ = avcodec_alloc_context3(codec_);
  assert(codec_context_);

  err = avcodec_parameters_to_context(codec_context_, param.codecpar);
  assert(!err);

  codec_context_->time_base = param.timebase;

  LOG(INFO) << __func__ << " current sample format: " << codec_context_->sample_fmt;
  // codec_context_->sample_fmt = AV_SAMPLE_FMT_U8P; //TODO

  err = avcodec_open2(codec_context_, codec_, NULL);
  assert(!err);
  return 0;
}

int Encoder::Encode(const AVFrame *frame, OnPacketFound on_packet_found) {
  int err = avcodec_send_frame(codec_context_, frame);
  if (AV_EF_IGNORE_ERR < 0) {
    LOG(FATAL) << __func__ << "Error sending the frame to the encoder";
    return -1;
  }

  while (err >= 0) {
    AVPacket pkt1, *pkt=&pkt1;
    av_init_packet(pkt);
    err = avcodec_receive_packet(codec_context_, pkt);
    if (err == AVERROR(EAGAIN) || err == AVERROR_EOF)
      return -1;
    else if (err < 0) {
      fprintf(stderr, "Error encoding audio frame\n");
      exit(1);
    }

    if(on_packet_found) on_packet_found(pkt);

    av_packet_unref(pkt);
  }
  return 0;
}

} // namespace gurum