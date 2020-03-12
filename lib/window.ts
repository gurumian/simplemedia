var {_Window} = require('bindings')('simplemedia');

export class Window {
  window: any;
  constructor(arg: {title: string; width: number; height: number}) {
    this.window = new _Window(arg)
  }

  createRenderer(): any {
    return this.window.createRenderer()
  }

  destroyRenderer(): void {
    this.window.destroyRenderer()
  }

  pollEvent(): object {
    return this.window.pollEvent()
  }

  get fullscreen(): boolean {
    return this.window.fullscreen
  }

  set fullscreen(arg: boolean) {
    this.window.fullscreen = arg
  }
}