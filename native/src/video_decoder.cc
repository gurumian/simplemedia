#include <assert.h>
#include "log_message.h"

#include "simplemedia/video_decoder.h"

namespace gurum {

// int VideoDecoder::decode(AVPacket *pkt, OnFrameFound on_frame_found) {
//   int err = 0;
//   assert(codec_context_);

//   err = avcodec_send_packet(codec_context_, pkt);
//   if(err < 0) {
//     LOG(ERROR) << __func__ << " E: avcodec_send_packet: " << err;
//     return err;
//   }

//   while (!err) {
//     err = avcodec_receive_frame(codec_context_, frame_);
//     if(err) continue;

//     if(timebase().num == 0 && timebase().den == 0) {
//       frame_->pts = frame_->best_effort_timestamp;
//     }
//     else {
//       const AVRational microseconds = {1, 1000000};
//       frame_->pts = av_rescale_q(frame_->pkt_dts, timebase(), microseconds);
//     }

//     if(on_frame_found) on_frame_found(frame_);
//   }
//   return 0;
// }

int VideoDecoder::width() {
  return codec_context_->width;
}

int VideoDecoder::height() {
  return codec_context_->height;
}

int VideoDecoder::pixelFormat() {
  return (int)codec_context_->pix_fmt;
}

} // namespace gurum