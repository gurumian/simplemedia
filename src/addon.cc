#include "simplemedia/config.h"
#include <napi.h>
#include "frame_wrap.h"
#include "source_wrap.h"
#include "audio_decoder_wrap.h"
#include "audio_renderer_wrap.h"
#include "video_decoder_wrap.h"
#include "video_renderer_wrap.h"
#include "window_wrap.h"

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
#if defined(USE_SDL2)
  Window::Init(env, exports);
  AudioRenderer::Init(env, exports);
  VideoRenderer::Init(env, exports);
#endif // USE_SDL2
  Frame::Init(env, exports);
  AudioDecoder::Init(env, exports);
  VideoDecoder::Init(env, exports);
  return Source::Init(env, exports);
}

NODE_API_MODULE(addon, InitAll)