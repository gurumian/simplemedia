var {_AudioRenderer, _VideoRenderer} = require('bindings')('simplemedia');

export abstract class Renderer {
  renderer: any;
  constructor() {}

  prepare(arg: any): void {
    this.renderer.prepare(arg)
  }

  render(frame: any): void {
    return this.renderer.render(frame.native)
  }

  set trace(enable: boolean) {
    this.renderer.trace = enable
  }
}

export class AudioRenderer extends Renderer {
  constructor() {
    super()
    this.renderer = new _AudioRenderer()
  }
  
  get volume(): number {
    return this.renderer.volume
  }

  set volume(vol: number) {
    this.renderer.volume = vol;
  }
}


export class VideoRenderer extends Renderer {
  constructor(surface: any) {
    super()
    this.renderer = new _VideoRenderer(surface)
  }

  resize(width: number, height: number): void {
    this.renderer.resize(width, height)
  }
}