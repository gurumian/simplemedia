import {Timer} from './timer'
import {Source} from './source'
import {AudioDecoder, VideoDecoder} from './decoder'
import {AudioRenderer, VideoRenderer} from './renderer'

export enum State {
  none,
  prepared,
  started,
  stopped,
  paused
}

const syncThreshold = 10000;
const delayStep = 100;

abstract class Element {
  renderer: any
  timer: Timer
  decoder: VideoDecoder | AudioDecoder
  isFirstFrame: boolean
  count: number
  pts: number
  constructor(fmt: any, source: Source, trace: boolean, renderer?:any) {
    this.init(fmt, source, trace, renderer)
    this.timer = new Timer()
    this.isFirstFrame = true
    this.count = 0
  }

  abstract init(fmt: object, source: Source, trace: boolean, renderer?:any): void
}

class Audio extends Element{
  constructor(fmt: any, source: any, trace: boolean, renderer?:any) {
    super(fmt, source, trace, renderer);
  }

  init(fmt: any, source: any, trace: boolean, renderer?:any): void {
    this.renderer = new AudioRenderer();
    this.renderer.trace = trace;

    let pid = source.audioPid;
    console.log(`audio index: ${pid}`);
    let pidchannel = source.requestPidChannel(pid);

    let decoder = new AudioDecoder();
    let strm = fmt.streams[pid].native;
    decoder.prepare(strm);
    decoder.pidchannel = pidchannel;
    decoder.trace = trace;
    decoder.done = false;
    this.renderer.prepare({
      samplerate: decoder.samplerate,
      channels: decoder.channels,
      channellayout: decoder.channellayout,
      sampleformat: decoder.sampleformat,
    });

    this.decoder = decoder;
  }
}

class Video extends Element{
  width: number;
  height: number;
  constructor(fmt: any, source: any, trace: boolean, renderer?:any) {
    super(fmt, source, trace, renderer);
  }

  init(fmt: any, source: any, trace: boolean, renderer: any): void {
    this.renderer = new VideoRenderer(renderer);

    let pid = source.videoPid;
    console.log(`video index: ${pid}`);
    let pidchannel = source.requestPidChannel(pid);

    let decoder = new VideoDecoder();
    let strm = fmt.streams[pid].native;
    decoder.prepare(strm);
    decoder.pidchannel = pidchannel;
    decoder.trace = trace;
    decoder.done = false;

    this.renderer.prepare({
      width: decoder.width,
      height: decoder.height,
      pixelformat: decoder.pixelformat,
    });

    this.decoder = decoder;
  }
}

export class MediaPlayer {
  trace: boolean;
  renderer: any;
  source: Source;
  state: number;
  audio: Audio;
  video: Video;
  onend: () => void;
  constructor({ renderer, trace }) {
    this.trace = trace || false;
    this.renderer = renderer || null;
    this.source = new Source();
    this.state = State.none;
  }

  async prepare() {
    let source = this.source;
    return new Promise((resolve, reject) => {
      let fmt = source.prepare();
      if(fmt) {
        // console.log(fmt);
        if(source.hasAudio) {
          this.audio = new Audio(fmt, source, true);
        }

        if(source.hasVideo && this.renderer) {
          this.video = new Video(fmt, source, true, this.renderer);
        }

        this.state = State.prepared;
        resolve(fmt);
      }
      else reject(`error: ${this.datasource}`);
    });
  }

  _ondecodedAudio(frame: any) {
    let delay: number = 0;
    let audio = this.audio;
    let video = this.video;
    if(frame) {
      let pts = frame.pts;

      if(audio.isFirstFrame) {
        audio.isFirstFrame = false;
        audio.timer.wait(0).then(() => {
          if(this.state == State.started) {
            audio.timer.update();
            audio.pts = pts;
            this._decode(audio.decoder);
          }
        });
      }
      else {
        let synced = true;
        if(this.hasVideoDecoder) {
          let diff = audio.pts - video.pts;
          if(diff > syncThreshold)
            synced = false;
        }

        delay = pts - audio.pts;
        audio.timer.wait(delay + (synced ? 0 : delayStep)).then(() => {
          if(this.state == State.started) {
            audio.pts = pts;
            this._decode(audio.decoder);
          }
        });
      }

      audio.renderer.render(frame);
      audio.count++;
    }
    else {
      console.log(`null packet, packet count: ${audio.count}`);
      audio.decoder.done = true;
      if(this.hasVideoDecoder && !video.decoder.done) return;
      if(this.onend) this.onend();
    }
  }

  _ondecodedVideo(frame: any) {
    let delay = 0;
    let audio = this.audio;
    let video = this.video;
    if(frame) {
      let pts = frame.pts;

      if(video.isFirstFrame) {
        video.isFirstFrame = false;
        video.timer.wait(0).then(() => {
          if(this.state == State.started) {
            video.timer.update();
            video.pts = pts;
            this._decode(video.decoder);
          }
        });
      }
      else {
        let synced = true;
        if(this.hasAudioDecoder) {
          let diff = video.pts - audio.pts;
          if(diff > syncThreshold) synced = false;
        }

        delay = pts - video.pts;
        video.timer.wait(delay + (synced ? 0 : delayStep)).then(() => {
          if(this.state == State.started) {
            video.pts = pts;
            this._decode(video.decoder);
          }
        });
      }

      if(video.renderer) {
        if(video.width != frame.width || video.height != frame.height) {
          video.width = frame.width;
          video.height = frame.height;
          video.renderer.resize(video.width, video.height);
        }

        video.renderer.render(frame);
      }
      video.count++;
    }
    else {
      console.log(`null packet, packet count: ${video.count}`);
      video.decoder.done = true;
      if(this.hasAudioDecoder && !audio.decoder.done) return;
      if(this.onend) this.onend();
    }
  }

  _ondecode({decoder, frame}) {
    if(this.audio.decoder == decoder)
      this._ondecodedAudio(frame);
    else
      this._ondecodedVideo(frame);
  }

  _decode(decoder: any) {
    decoder.decode()
    .then((frames: any) => {
      if(frames) {
        frames.map((frame: any) => {
          this._ondecode({
            decoder: decoder,
            frame: frame,
          })
        })
      }
      else { // eos
        this._ondecode({
          decoder: decoder,
          frame: null,
        })
      }
    })
  }

  /**
   * @param {() => void} onend - notified on EOF
   */
  set onended(onend: () => void) {
    this.onend = onend;
  }

  /**
   * 
   * start the player
   */
  start(): void {
    this.state = State.started;
    this.source.start();

    if(this.hasAudioDecoder)
      this._decode(this.audio.decoder);

    if(this.hasVideoDecoder)
      this._decode(this.video.decoder);
  }

  /**
   * 
   * stop the player
   */
  stop(): void {
    this.state = State.stopped;

    if(this.hasVideoDecoder)
      this.video.decoder.stop();

    if(this.hasAudioDecoder)
      this.audio.decoder.stop();

    this.source.stop();
  }

  /**
   * 
   * pause the player
   */
  pause(): void {
    this.state = State.paused;
    this.source.pause();

    if(this.hasAudioDecoder)
      this.audio.decoder.pause();

    if(this.hasVideoDecoder)
      this.video.decoder.pause();

    this.position = this.position;
  }

  /**
   * 
   * start the player
   */
  resume(): void {
    this.start();
  }

  /**
   * @param {string} datasource
   */
  set datasource(datasource: string) {
    this.source.datasource = datasource;
  }

  /**
   * 
   * @return current position
   */
  get position(): number {
    if(this.hasVideoDecoder) return this.video.pts;
    if(this.hasAudioDecoder) return this.audio.pts;
    return 0;
  }

  /**
   * 
   * @param {number} pos - new position
   */
  set position(pos: number) {
    this.source.seek({
      pos: pos,
      backward: (pos < this.position) ? true : false,
      callback: (() => {
        if(this.hasVideoDecoder) {
          this.video.decoder.flush();
          this.video.isFirstFrame = true;
        }

        if(this.hasAudioDecoder) {
          this.audio.decoder.flush();
          this.audio.isFirstFrame = true;
        }
      }),
    })
  }

  /**
   * 
   * @return current volume
   */
  get volume() : number{
    return this.audio.renderer.volume;
  }

  /**
   * 
   * @param {number} vol - 0.0 ~ 1.0
   */
  set volume(vol: number) {
    this.audio.renderer.volume = vol;
  }

  /**
   * 
   * @return duration
   */
  get duration(): number {
    return this.source.duration;
  }


  /**
   * 
   * @return true if a video decoder exists
   */
  get hasVideoDecoder(): boolean  {
    return !!(this.video && this.video.decoder);
  }

  /**
   * 
   * @return true if a audio decoder exists
   */
  get hasAudioDecoder(): boolean {
    return !!(this.audio && this.audio.decoder);
  }
}

export default MediaPlayer