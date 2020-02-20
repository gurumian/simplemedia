const {Source, AudioDecoder, AudioRenderer, VideoRenderer, VideoDecoder} = require('bindings')('simplemedia');
const Timer = require('./timer');

const State = Object.freeze({
  'none':0,
  'prepared':1,
  'started':2,
  'stopped':3,
  'paused':4,
});

module.exports = class MediaPlayer {
  constructor({ renderer: renderer, trace: trace }) {
    if(!trace) trace = false;

    this.trace = trace;

    this.source = new Source();
    this.audio = {};
    this.video = {};
    
    this.audio.renderer = new AudioRenderer();
    if(renderer) {
      this.renderer = renderer;
      this.video.renderer = new VideoRenderer(renderer);
      this.video.renderer.trace = this.trace;
    }
    
    this.audio.count = 0;
    this.video.count = 0;

    this.source.trace = trace;
    this.audio.renderer.trace = trace;

    this.audio.isFirstFrame = true;
    this.video.isFirstFrame = true;

    this.State = State;
    this.state = State.none;
  }

  _prepareAudio(fmt, source) {
    let pid = source.audioPid;
    console.log(`audio index: ${pid}`);
    let pidchannel = source.requestPidChannel({
      pid : pid,
    });

    let decoder = new AudioDecoder();
    decoder.prepare(fmt.streams[pid]['native']);
    decoder.pidchannel = pidchannel;
    decoder.trace = this.trace;
    decoder.done = false;
    this.audio.renderer.prepare({
      samplerate: decoder.samplerate,
      channels: decoder.channels,
      channellayout: decoder.channellayout,
      sampleformat: decoder.sampleformat,
    });

    this.audio.timer = new Timer();
    this.audio.decoder = decoder;
  }

  _prepareVideo(fmt, source) {
    let pid = source.videoPid;
    console.log(`video index: ${pid}`);
    let pidchannel = source.requestPidChannel({
      pid : pid,
    });

    let decoder = new VideoDecoder();
    decoder.prepare(fmt.streams[pid]['native']);
    decoder.pidchannel = pidchannel;
    decoder.trace = this.trace;
    decoder.done = false;

    this.video.renderer.prepare({
      width: decoder.width,
      height: decoder.height,
      pixelformat: decoder.pixelformat,
    });

    this.video.timer = new Timer();
    this.video.decoder = decoder;
  }

  async prepare() {
    let source = this.source;
    return new Promise((resolve, reject) => {
      let fmt = source.prepare();
      if(fmt) {
        console.log(fmt);
        if(source.hasAudio) {
          this._prepareAudio(fmt, source);
        }

        if(source.hasVideo && this.video.renderer) {
          this._prepareVideo(fmt, source);
        }

        this.state = State.prepared;
        resolve(fmt);
      }
      else {
        reject();
      }
    });  
  }

  _ondecodedAudio(frame) {
    let delay = 0;
    let audio = this.audio;
    if(frame) {
      let pts = frame.pts;

      if(audio.isFirstFrame) {
        audio.isFirstFrame = false;
        audio.timer.wait(0).then(() => {
          if(this.state == State.started) {
            audio.timer.update();
            audio.pts = pts;
            this._decodeAudio();
          }
        });
      }
      else {
        const sync_threshold = 5000;
        const delay_step = 100;
        let synced = true;
        if(this.source.hasVideo) {
          let diff = audio.pts - this.video.pts;
          if(diff > sync_threshold)
            synced = false;
        }

        delay = pts - audio.pts;
        audio.timer.wait(delay + (synced ? 0 : delay_step)).then(() => {
          if(this.state == State.started) {
            audio.pts = pts;
            this._decodeAudio();
          }
        });
      }

      audio.renderer.render(frame.native);
      audio.count++;
    }
    else {
      console.log(`null packet, packet count: ${audio.count}`);
      audio.decoder.done = true;
      if(this.video.decoder) {
        if(this.video.decoder.done) {
          if(this.onend) this.onend();
        }
      }
      else {
        if(this.onend) this.onend();
      }
    }
  }

  _ondecodedVideo(frame) {
    let delay = 0;
    let video = this.video;
    if(frame) {
      let pts = frame.pts;

      if(video.isFirstFrame) {
        video.isFirstFrame = false;
        video.timer.wait(0).then(() => {
          if(this.state == State.started) {
            video.timer.update();
            video.pts = pts;
            this._decodeVideo();
          }
        });
      }
      else {
        const syncThreshold = 5000;
        const delayStep = 100;
        let synced = true;
        if(this.source.hasAudio) {
          let diff = video.pts - this.audio.pts;
          if(diff > syncThreshold)
            synced = false;
        }

        delay = pts - video.pts;
        video.timer.wait(delay + (synced ? 0 : delayStep)).then(() => {
          if(this.state == State.started) {
            video.pts = pts;
            this._decodeVideo();
          }
        });
      }

      if(video.width != frame.width || video.height != frame.height) {
        video.width = frame.width;
        video.height = frame.height;
        video.renderer.resize(video.width, video.height);
      }

      video.renderer.render(frame.native);
      video.count++;
    }
    else {
      console.log(`null packet, packet count: ${video.count}`);
      video.decoder.done = true;
      if(this.audio.decoder) {
        if(this.audio.decoder.done) {
          if(this.onend) this.onend();
        }
      }
      else {
        if(this.onend) this.onend();
      }
    }
  }

  _decodeAudio() {
    let done = false;
    let decoder = this.audio.decoder;
    while(!done) {
      done = decoder.decode(frame => this._ondecodedAudio(frame));
    }
  }

  _decodeVideo() {
    let done = false;
    let decoder = this.video.decoder;
    while(!done) {
      done = decoder.decode(frame => this._ondecodedVideo(frame));
    }
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

    if(this.audio.decoder)
      this._decodeAudio();

    if(this.video.decoder)
      this._decodeVideo();
  }

  stop() {
    this.state = State.stopped;

    if(this.video.decoder)
      this.video.decoder.stop();

    if(this.audio.decoder)
      this.audio.decoder.stop();

    this.source.stop();
  }

  pause() {
    this.state = State.paused;
    this.source.pause();

    if(this.audio.decoder)
      this.audio.decoder.pause();

    if(this.video.decoder)
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
    return this.video.pts || this.audio.pts || 0;
  }

  set position(pos) {
    this.source.seek({
      pos: pos,
      backward: (pos < this.position) ? true : false,
      callback: (() => {
        if(this.video.decoder) {
          this.video.decoder.flush();
        }

        if(this.audio.decoder) {
          this.audio.decoder.flush();
        }

        this.video.isFirstFrame = this.audio.isFirstFrame = true;
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
}