'use strict'

const {Source, AudioDecoder, AudioRenderer, VideoRenderer, VideoDecoder} = require('bindings')('simplemedia');
const Timer = require('./timer');

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
  }

  _prepareAudio(fmt, source) {
    let pid = source.audioPid;
    console.log(`audio index: ${pid}`);
    let pidchannel = source.requestPidChannel({
      pid : pid,
    });

    this.audio.decoder = new AudioDecoder();
    this.audio.decoder.prepare(fmt.streams[pid]['native']);
    this.audio.decoder.pidchannel = pidchannel;
    this.audio.timer = new Timer();
    this.audio.decoder.trace = this.trace;
    this.audio.decoder.done = false;

    this.audio.renderer.prepare({
      samplerate: this.audio.decoder.samplerate,
      channels: this.audio.decoder.channels,
      channellayout: this.audio.decoder.channellayout,
      sampleformat: this.audio.decoder.sampleformat,
    });
  }

  _prepareVideo(fmt, source) {
    let pid = source.videoPid;
    console.log(`video index: ${pid}`);
    let pidchannel = source.requestPidChannel({
      pid : pid,
    });

    this.video.decoder = new VideoDecoder();
    this.video.decoder.prepare(fmt.streams[pid]['native']);
    this.video.decoder.pidchannel = pidchannel;
    this.video.timer = new Timer();
    this.video.decoder.trace = this.trace;
    this.video.decoder.done = false;

    this.video.renderer.prepare({
      width: this.video.decoder.width,
      height: this.video.decoder.height,
      pixelformat: this.video.decoder.pixelformat,
    });
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

        resolve(fmt);
      }
      else {
        reject();
      }
    });  
  }

  _ondecodedAudio(frame) {
    var delay = 0;
    if(frame) {
      let pts = frame.pts;

      if(this.audio.isFirstFrame) {
        this.audio.pts = pts;
        this.audio.timer.update();
        this.audio.isFirstFrame = false;
        this.audio.timer.wait(0).then(() => {
          this._decodeAudio();
        });
      }
      else {
        const sync_threshold = 5000;
        const delay_step = 100;
        var synced = true;
        if(this.source.hasVideo) {
          let diff = this.audio.pts - this.video.pts;
          if(diff > sync_threshold)
            synced = false;
        }

        delay = pts - this.audio.pts;
        this.audio.pts = pts;
        this.audio.timer.wait(delay + (synced ? 0 : delay_step)).then(() => {
          this._decodeAudio();
        });;
      }

      this.audio.renderer.render(frame.native);
      this.audio.count++;
    }
    else {
      console.log(`null packet, packet count: ${this.audio.count}`);
      this.audio.decoder.done = true;
      if(this.video.decoder) {
        if(this.video.decoder.done) setTimeout(()=>{
          if(this.onend)
            this.onend();
        });
      }
      else {
        setTimeout(()=>{
          if(this.onend)
            this.onend();
        });
      }
    }
  }

  _decodeAudio() {
    if(! this.audio.decoder) return;

    var done = false;
    while(!done) {
      done = this.audio.decoder.decode(frame => this._ondecodedAudio(frame));
    }
  }

  _ondecodedVideo(frame) {
    var delay = 0;
    if(frame) {
      let pts = frame.pts;

      if(this.video.isFirstFrame) {
        this.video.pts = pts;
        this.video.isFirstFrame = false;
        this.audio.timer.wait(0).then(() => {
          this._decodeVideo();
        });
      }
      else {
        const syncThreshold = 5000;
        const delayStep = 100;
        var synced = true;
        if(this.source.hasAudio) {
          let diff = this.video.pts - this.audio.pts;
          if(diff > syncThreshold)
            synced = false;
        }

        delay = pts - this.video.pts;
        this.video.pts = pts;
        this.video.timer.wait(delay + (synced ? 0 : delayStep)).then(() => {
          this._decodeVideo();
        });
      }

      if(this.video.width != frame.width || this.video.height != frame.height) {
        this.video.width = frame.width;
        this.video.height = frame.height;
        this.video.renderer.resize(this.video.width, this.video.height);
      }

      this.video.renderer.render(frame.native);
      this.video.count++;
    }
    else {
      console.log(`null packet, packet count: ${this.video.count}`);
      this.video.decoder.done = true;
      if(this.audio.decoder) {
        if(this.audio.decoder.done) setTimeout(()=>{
          if(this.onend)
            this.onend();
        });
      }
      else {
        setTimeout(()=>{
          if(this.onend)
            this.onend();
        });
      }
    }
  }

  _decodeVideo() {
    if(! this.video.decoder) return;

    var done = false;
    while(!done) {
      done = this.video.decoder.decode(frame => this._ondecodedVideo(frame));
    }
  }
  /**
   * @param {() => void} onend
   */
  set onended(onend) {
    this.onend = onend;
  }

  start() {
    this.source.start();
    setTimeout(()=>{
      this._decodeAudio();
    });

    setTimeout(()=>{
      this._decodeVideo();
    });
  }

  stop() {
    if(this.video.decoder)
      this.video.decoder.stop();

    if(this.audio.decoder)
      this.audio.decoder.stop();

    this.source.stop();
  }

  pause() {
    this.source.pause();
    if(this.audio.decoder)
      this.audio.decoder.pause();

    if(this.video.decoder)
      this.video.decoder.pause();
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
    if(this.video) {
      return this.video.pts;
    }
    return this.audio.pts;
  }

  set position(pos) {
    console.log(pos);
    this.source.seek({
      pos: pos,
      backward: (pos < this.position) ? true : false,
      callback: (() => {
        if(this.video)
          this.video.decoder.flush();
        if(this.audio)
          this.audio.decoder.flush();

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