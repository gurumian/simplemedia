var {_Source} = require('bindings')('simplemedia');

export class Source {
  private _datasource: string
  private source: any
  constructor() {
    this.source = new _Source()    
  }

  prepare(): object {
    return this.source.prepare()
  }

  start(): void {
    this.source.start()
  }

  stop(): void {
    this.source.stop()
  }

  pause(): void {
    this.source.pause()
  }

  seek(arg: {pos: number; backward: boolean; callback: ()=>void;}) {
    this.source.seek(arg)
  }

  requestPidChannel(pid: number): object {
    return this.source.requestPidChannel({
      pid: pid
    })
  }

  get datasource(): string {
    return this._datasource
  }

  set datasource(datasource: string) {
    this._datasource = datasource
    this.source.datasource = datasource;
  }

  get hasAudio(): boolean {
    return this.source.hasAudio
  }

  get hasVideo(): boolean {
    return this.source.hasVideo
  }

  get videoPid(): boolean {
    return this.source.videoPid
  }

  get audioPid(): boolean {
    return this.source.audioPid
  }

  get duration(): number {
    return this.source.duration
  }

  set trace(enable: boolean) {
    this.source.trace = enable
  }

  get trace(): boolean {
    return this.source.trace
  }
}

export default Source