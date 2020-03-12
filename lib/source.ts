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

  seek(arg: {pos: number; backword: boolean; callback: ()=>void;}) {
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
}

export default Source




// InstanceMethod("prepare", &Source::Prepare),
// InstanceMethod("start", &Source::Start),
// InstanceMethod("stop", &Source::Stop),
// InstanceMethod("pause", &Source::Pause),
// InstanceMethod("seek", &Source::Seek),
// InstanceMethod("requestPidChannel", &Source::RequestPidChannel),
// InstanceMethod("findStream", &Source::FindStream),
// InstanceAccessor("datasource", &Source::dataSource, &Source::SetDataSource),
// InstanceAccessor("audioPid", &Source::audioPid, nullptr),
// InstanceAccessor("hasAudio", &Source::hasAudio, nullptr),
// InstanceAccessor("videoPid", &Source::videoPid, nullptr),
// InstanceAccessor("hasVideo", &Source::hasVideo, nullptr),
// InstanceAccessor("trace", &Source::log_enabled, &Source::EnableLog),
// InstanceAccessor("duration", &Source::duration, nullptr),