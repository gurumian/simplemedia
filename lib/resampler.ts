var {_Resampler} = require('bindings')('simplemedia');


export class Resampler {
  resampler: any
  constructor(arg: {samplerate: number; channels: number; channellayout: number; sampleformat: number;}) {
    this.resampler = new _Resampler(arg)
  }

  resample(frame: any) {
    return this.resampler.resample()
  }
}
