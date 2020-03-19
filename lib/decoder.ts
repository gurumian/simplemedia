var {_AudioDecoder, _VideoDecoder} = require('bindings')('simplemedia');
import {Resampler} from './resampler'

export abstract class Decoder {
  decoder: any
  done: boolean
  constructor() {}

  prepare(strm: any): void {
    this.decoder.prepare(strm.native)
  }

  start(): void {
    this.decoder.start()
  }

  stop(): void {
    this.decoder.stop()
  }

  pause(): void {
    this.decoder.pause()
  }

  decode(): Promise<any> {
    return this.decoder.decode()
  }

  flush(): void {
    this.decoder.flush()
  }

  set pidchannel(arg: any) {
    this.decoder.pidchannel = arg
  }

  set trace(arg: boolean) {
    this.decoder.trace = arg
  }

  get trace(): boolean {
    return this.decoder.trace
  }
}



export class AudioDecoder extends Decoder {
  constructor() {
    super()
    this.decoder = new _AudioDecoder()
  }

  get sampleformat(): number {
    return this.decoder.sampleformat
  }

  get samplerate(): number {
    return this.decoder.samplerate
  }

  get channels(): number {
    return this.decoder.channels
  }

  get channellayout(): number {
    return this.decoder.channellayout
  }

  createResampler(arg: {sampleformat: number; samplerate: number; channels: number; channellayout: number}) {
    let resampler = new Resampler()
    resampler.prepare({
      src: {
        samplerate: this.decoder.samplerate,
        channels: this.decoder.channels,
        channellayout: this.decoder.channellayout,
        sampleformat: this.decoder.sampleformat,
      },
      dst: arg,
    })
    return resampler
  }
}

export class VideoDecoder extends Decoder {
  constructor() {
    super()
    this.decoder = new _VideoDecoder()
  }

  get width(): number {
    return this.decoder.width
  }

  get height(): number {
    return this.decoder.height
  }

  get pixelformat(): number {
    return this.decoder.pixelformat
  }
}

