#include <napi.h>
#include "source_wrap.h"
// #include "pid_channel_wrap.h"
#include "audio_decoder_wrap.h"
#include "audio_renderer_wrap.h"
// #include "format_context_wrap.h"

// Napi::String Method(const Napi::CallbackInfo& info) {
//   Napi::Env env = info.Env();
//   return Napi::String::New(env, "world");
// }

// Napi::Object Init(Napi::Env env, Napi::Object exports) {
//   exports.Set(Napi::String::New(env, "hello"),
//               Napi::Function::New(env, Method));
//   return exports;
// }

// NODE_API_MODULE(hello, Init)


Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
  AudioRenderer::Init(env, exports);
  AudioDecoder::Init(env, exports);
  return Source::Init(env, exports);
}

NODE_API_MODULE(addon, InitAll)
