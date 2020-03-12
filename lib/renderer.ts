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

  set trace(enable: boolean) {
  }
}


export class VideoRenderer extends Renderer {
  constructor(surface: any) {
    super()
    this.renderer = new _VideoRenderer(surface)
  }
}