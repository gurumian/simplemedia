var {_Resampler} = require('bindings')('simplemedia');


export class Resampler {
  resampler: any
  constructor() {
  }

  prepare(arg: {samplerate: number; channels: number; channellayout: number; sampleformat: number;}): void {
    this.resampler = new _Resampler(arg)
  }

  resample(frame: any): ArrayBuffer {
    return this.resampler.resample(frame.native)
  }
}