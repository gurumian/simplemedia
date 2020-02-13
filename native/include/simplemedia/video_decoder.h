#ifndef GURUM_VIDEO_DECODER_H_
#define GURUM_VIDEO_DECODER_H_

#include <simplemedia/config.h>
#include "frame_decoder.h"

namespace gurum {

class VideoDecoder : public FrameDecoder {
public:
	int width();
	int height();
	int pixelFormat();

	virtual AVMediaType MediaType() const override {return AVMEDIA_TYPE_VIDEO;}
};

} // namespace gurum

#endif // GURUM_VIDEO_DECODER_H_