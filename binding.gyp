{
  "targets": [
    {
      "target_name": "simplemedia",
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "sources": [ 
        "src/sdl/sdl_audio_renderer.cc",
        "src/sdl/sdl_subtitle_renderer.cc",
        "src/sdl/sdl_video_renderer.cc",
        "src/audio_decoder.cc",
        "src/audio_encoder.cc",
        "src/decoder.cc",
        "src/encoder.cc",
        "src/frame_decoder.cc",
        "src/media_player.cc",
        "src/packet_pool.cc",
        "src/pid_channel.cc",
        "src/simple_thread.cc",
        "src/source.cc",
        "src/subtitle_decoder.cc",
        "src/timer.cc",
        "src/video_decoder.cc",
        "node/src/addon.cc",
        "node/src/frame_wrap.cc",
        "node/src/source_wrap.cc",
        "node/src/audio_decoder_wrap.cc",
        "node/src/audio_renderer_wrap.cc",
        "node/src/video_decoder_wrap.cc",
        "node/src/video_renderer_wrap.cc",
      ],
      'link_settings': {
        'libraries': [
          '-lavformat',
          '-lavcodec',
          '-lavutil',
          '-lswresample',
          '-lSDL2'
        ],
      },
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "src",
        "include",
        "include/simplemedia",
        "include/simplemedia/sdl"
      ],
      'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ],
    }
  ]
}
