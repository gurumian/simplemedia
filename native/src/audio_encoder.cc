#include "simplemedia/audio_encoder.h"

#include "log_message.h"
extern "C" {
#include <libavutil/base64.h>
}

namespace gurum {

int AudioEncoder::Encode(AVFrame *frame, OnPacketFound on_packet_found) {
#if (LIBAVCODEC_VERSION_MAJOR > 57)
  int ret = avcodec_send_frame(codec_context_, frame);
  if (ret < 0) {
      fprintf(stderr, "Error sending the frame to the encoder\n");
      exit(1);
  }

  /* read all the available output packets (in general there may be any
   * number of them */
  while (ret >= 0) {
    AVPacket pkt1, *pkt=&pkt1;
    av_init_packet(pkt);
    ret = avcodec_receive_packet(codec_context_, pkt);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        return -1;
    else if (ret < 0) {
        fprintf(stderr, "Error encoding audio frame\n");
        exit(1);
    }

    if(on_packet_found) on_packet_found(pkt);
    av_packet_unref(pkt);
  }
#else
  // TODO:
#endif
  return 0;
}

int AudioEncoder::samplerate() {
  assert(codec_context_);
  return codec_context_->sample_rate;
}

int AudioEncoder::sampleFormat() {
  assert(codec_context_);
  return codec_context_->sample_fmt;
}

int AudioEncoder::channels() {
  assert(codec_context_);
  return codec_context_->channels;
}

int64_t AudioEncoder::channellayout() {
  assert(codec_context_);
  return codec_context_->channel_layout;
}

AVRational AudioEncoder::timebase() {
  return codec_context_->time_base;
}

} // namespace gurum