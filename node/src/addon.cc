#include <napi.h>
#include "frame_wrap.h"
#include "source_wrap.h"
#include "audio_decoder_wrap.h"
#include "audio_renderer_wrap.h"

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
  Frame::Init(env, exports);
  AudioRenderer::Init(env, exports);
  AudioDecoder::Init(env, exports);
  return Source::Init(env, exports);
}

NODE_API_MODULE(addon, InitAll)
