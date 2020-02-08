'use strict'

const {Source, AudioDecoder, AudioRenderer, VideoRenderer, VideoDecoder} = require('bindings')('simplemedia');
const adjust = 0;
module.exports = class MediaPlayer {
  constructor({ renderer: renderer, trace: trace }) {
    if(!trace) trace = false;

    this.trace = trace;

    this.source = new Source();
    this.audio = {};
    this.video = {};
    this.renderer = renderer; // native renderer
    
    this.audio.renderer = new AudioRenderer();
    if(renderer) {
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
    this.audio.decoder.trace = this.trace;

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
    this.video.decoder.trace = this.trace;

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

        if(source.hasVideo) {
          this._prepareVideo(fmt, source);
        }

        resolve(fmt);
      }
      else {
        reject();
      }
    });  
  }

  _decodeAudio() {
    this.audio.decoder.decode(frame => {
      var delay = 0;
      if(frame) {
        if(this.audio.isFirstFrame) {
          this.audio.pts = frame.pts;
          this.audio.isFirstFrame = false;
        }
        else {
          delay = frame.pts - this.audio.pts;
          this.audio.pts = frame.pts;
        }

        this.audio.renderer.render(frame.native);
        this.audio.count++;
       
        delay = Math.floor(delay / 1000) - adjust;
        setTimeout(()=>{
          this._decodeAudio();
        }, delay);
      }
      else {
        console.log(`null packet, packet count: ${this.audio.count}`);
        setTimeout(()=>{
          if(this.onend)
            this.onend();
        });
      }
    });
  }

  _ondecodedVideo(frame) {
    var delay = 0;
    if(frame) {
      if(this.video.isFirstFrame) {
        this.video.pts = frame.pts;
        this.video.isFirstFrame = false;
      }
      else {
        const sync_threshold = 5000;
        const delay_step = 100;
        var synced = true;
        if(this.source.hasAudio && this.source.hasVideo) {
          let diff = this.video.pts - this.audio.pts;
          if(diff > sync_threshold)
            synced = false;
        }

        delay = frame.pts - this.video.pts;
        if(! synced)
          delay += delay_step;

        this.video.pts = frame.pts;
      }

      if(this.video.width != frame.width || this.video.height != frame.height) {
        this.video.width = frame.width;
        this.video.height = frame.height;
        this.video.renderer.resize(this.video.width, this.video.height);
      }

      this.video.renderer.render(frame.native);
      this.video.count++;

      delay = Math.floor(delay / 1000) - adjust;
      setTimeout(()=>{
        this._decodeVideo();
      }, delay);
    }
    else {
      console.log(`null packet, packet count: ${this.video.count}`);
    }
  }

  _decodeVideo() {
    try {
      this.video.decoder.decode(frame => this._ondecodedVideo(frame));
    }
    catch(err) {
      setTimeout(()=>{
        this._decodeVideo();
      });
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