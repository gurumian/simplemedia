var {_AudioDecoder, _VideoDecoder} = require('bindings')('simplemedia');

export abstract class Decoder {
  decoder: any;
  constructor() {}

  prepare(strm: any) {
    this.decoder.prepare(strm)
  }

  start() {
    this.decoder.start()
  }

  stop() {
    this.decoder.stop()
  }

  pause() {
    this.decoder.pause()
  }

  decode() {
    return this.decoder.decode()
  }

  flush() {
    this.decoder.flush()
  }

  set pidchannel(arg: any) {
    this.decoder.pidchannel = arg;
  }

  set done(arg: boolean) {

  }

  set trace(arg: boolean) {

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

