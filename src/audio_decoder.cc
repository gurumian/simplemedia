#include "simplemedia/config.h"
#include "simplemedia/audio_decoder.h"
#include "log_message.h"
extern "C" {
#include <libavutil/base64.h>
}

namespace gurum {

int AudioDecoder::decode(AVPacket *pkt, OnFrameFound on_frame_found) {
  // int decoded = 0;
  // int got = 0;
  int err = 0;
  assert(codec_context_);
  assert(frame_);

  err = avcodec_send_packet(codec_context_, pkt);
  if(err < 0) {
    LOG(INFO) << __func__ << " E: avcodec_send_packet: " << err;
    return err;
  }

  while (err>=0) {
    err = avcodec_receive_frame(codec_context_, frame_);
    if (!err)  {
#if 0
      AVRational tb = (AVRational){1, frame_->sample_rate};
      if (frame_->pts != AV_NOPTS_VALUE) {
        frame_->pts = av_rescale_q(frame_->pts, av_codec_get_pkt_timebase(codec_context_), tb);
      }
      else {
        LOG(ERROR) << __func__ << " error!";
        assert(1);
      }
#else
      const AVRational microseconds = {1, 1000000};
      if(timebase().num == 0 && timebase().den == 0) {
        frame_->pts = frame_->best_effort_timestamp;
      }
      else {
        frame_->pts = av_rescale_q(frame_->pkt_dts, timebase(), microseconds);
      }
#endif

      if(on_frame_found) on_frame_found(frame_);
    }
  }
  return 0;
}

int AudioDecoder::samplerate() {
	assert(codec_context_);
	return codec_context_->sample_rate;
}

AVSampleFormat AudioDecoder::sampleFormat() {
	assert(codec_context_);
	return codec_context_->sample_fmt;
}

int AudioDecoder::channels() {
	assert(codec_context_);
	return codec_context_->channels;
}

int64_t AudioDecoder::channellayout() {
	assert(codec_context_);
	return codec_context_->channel_layout;
}

AVRational AudioDecoder::timebase() {
  return stream_->time_base;
}

} // namespace gurum
