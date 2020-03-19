var {_Resampler} = require('bindings')('simplemedia');

export enum SampleFormat {
  none = -1,
  u8,
  s16,
  s32,
  flt,
  dbl,
  u8p,
  s16p,
  s32p,
  fltp,
  dblp
}

export enum ChannelLayout {
  mono = 0x00000004,
  stereo = 0x00000001 |  0x00000002
}

export class Resampler {
  resampler: any
  constructor() {
  }

  prepare(arg: {
    src: {samplerate: number; channels: number; channellayout: number; sampleformat: number;},
    dst: {samplerate: number; channels: number; channellayout: number; sampleformat: number;}
  }): void {
    this.resampler = new _Resampler(arg)
  }

  resample(frame: any): ArrayBuffer {
    return this.resampler.resample(frame.native)
  }
}

// export default Resampler