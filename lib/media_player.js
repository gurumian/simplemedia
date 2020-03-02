const {Source, AudioDecoder, AudioRenderer, VideoRenderer, VideoDecoder} = require('bindings')('simplemedia');
const Timer = require('./timer');

const State = Object.freeze({
  'none':0,
  'prepared':1,
  'started':2,
  'stopped':3,
  'paused':4,
});

const syncThreshold = 5000;
const delayStep = 100;

class MediaPlayer {
  constructor({ renderer: renderer, trace: trace }) {
    this.trace = trace || false;
    this.renderer = renderer || null;
    this.source = new Source();

    this.State = State;
    this.state = State.none;
  }

  _prepareAudio(fmt, source) {
    let audio = {};

    audio.renderer = new AudioRenderer();
    audio.renderer.trace = this.trace;

    let pid = source.audioPid;
    console.log(`audio index: ${pid}`);
    let pidchannel = source.requestPidChannel({
      pid : pid,
    });

    let decoder = new AudioDecoder();
    let strm = fmt.streams[pid].native;
    decoder.prepare(strm);
    decoder.pidchannel = pidchannel;
    decoder.trace = this.trace;
    decoder.done = false;
    audio.renderer.prepare({
      samplerate: decoder.samplerate,
      channels: decoder.channels,
      channellayout: decoder.channellayout,
      sampleformat: decoder.sampleformat,
    });

    audio.timer = new Timer();
    audio.decoder = decoder;
    audio.isFirstFrame = true;
    audio.count = 0;
    return audio;
  }

  _prepareVideo(fmt, source) {
    let video = {};

    if(this.renderer) {
      let renderer = new VideoRenderer(this.renderer);
      renderer.trace = this.trace;
      video.renderer = renderer;
    }

    let pid = source.videoPid;
    console.log(`video index: ${pid}`);
    let pidchannel = source.requestPidChannel({
      pid : pid,
    });

    let decoder = new VideoDecoder();
    let strm = fmt.streams[pid].native;
    decoder.prepare(strm);
    decoder.pidchannel = pidchannel;
    decoder.trace = this.trace;
    decoder.done = false;

    video.renderer.prepare({
      width: decoder.width,
      height: decoder.height,
      pixelformat: decoder.pixelformat,
    });

    video.timer = new Timer();
    video.decoder = decoder;
    video.isFirstFrame = true;
    video.count = 0;
    return video;
  }

  async prepare() {
    let source = this.source;
    return new Promise((resolve, reject) => {
      let fmt = source.prepare();
      if(fmt) {
        // console.log(fmt);
        if(source.hasAudio) {
          this.audio = this._prepareAudio(fmt, source);
        }

        if(source.hasVideo && this.renderer) {
          this.video = this._prepareVideo(fmt, source);
        }

        this.state = State.prepared;
        resolve(fmt);
      }
      else reject(`error: ${this.datasource}`);
    });
  }

  _ondecodedAudio(frame) {
    let delay = 0;
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
        if(this.hasVideoDecoder()) {
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

      audio.renderer.render(frame.native);
      audio.count++;
    }
    else {
      console.log(`null packet, packet count: ${audio.count}`);
      audio.decoder.done = true;
      if(this.hasVideoDecoder() && !video.decoder.done) return;
      if(this.onend) this.onend();
    }
  }

  _ondecodedVideo(frame) {
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
        if(this.hasAudioDecoder()) {
          let diff = video.pts - audio.pts;
          if(diff > syncThreshold)
            synced = false;
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

        video.renderer.render(frame.native);
      }
      video.count++;
    }
    else {
      console.log(`null packet, packet count: ${video.count}`);
      video.decoder.done = true;
      if(this.hasAudioDecoder() && !audio.decoder.done) return;
      if(this.onend) this.onend();
    }
  }

  _ondecode({decoder, frame}) {
    if(this.audio.decoder == decoder)
      this._ondecodedAudio(frame);
    else
      this._ondecodedVideo(frame);
  }

  _decode(decoder) {
    decoder.decode()
    .then(frame => {
      if(frame === undefined) { // retry
        setTimeout(()=>this._decode(decoder))
        return;
      }

      this._ondecode({
        decoder: decoder,
        frame: frame,
      })
    })
  }

  /**
   * @param {() => void} onend
   */
  set onended(onend) {
    this.onend = onend;
  }

  start() {
    this.state = State.started;
    this.source.start();

    if(this.hasAudioDecoder())
      this._decode(this.audio.decoder);

    if(this.hasVideoDecoder())
      this._decode(this.video.decoder);
  }

  stop() {
    this.state = State.stopped;

    if(this.hasVideoDecoder())
      this.video.decoder.stop();

    if(this.hasAudioDecoder())
      this.audio.decoder.stop();

    this.source.stop();
  }

  pause() {
    this.state = State.paused;
    this.source.pause();

    if(this.hasAudioDecoder())
      this.audio.decoder.pause();

    if(this.hasVideoDecoder())
      this.video.decoder.pause();

    this.position = this.position;
  }

  resume() {
    this.start();
  }

  /**
   * @param {string} datasource
   */
  set datasource(datasource) {
    this.source.datasource = datasource;
  }

  get position() {
    if(this.hasVideoDecoder()) return this.video.pts;
    if(this.hasAudioDecoder()) return this.audio.pts;
    return 0;
  }

  set position(pos) {
    this.source.seek({
      pos: pos,
      backward: (pos < this.position) ? true : false,
      callback: (() => {
        if(this.hasVideoDecoder()) {
          this.video.decoder.flush();
          this.video.isFirstFrame = true;
        }

        if(this.hasAudioDecoder()) {
          this.audio.decoder.flush();
          this.audio.isFirstFrame = true;
        }
      }),
    });
  }

  get volume() {
    return this.audio.renderer.volume;
  }

  set volume(vol) {
    this.audio.renderer.volume = vol;
  }

  get duration() {
    return this.source.duration;
  }


  hasVideoDecoder() {
    return (this.video && this.video.decoder);
  }

  hasAudioDecoder() {
    return (this.audio && this.audio.decoder);
  }
}

module.exports = MediaPlayer