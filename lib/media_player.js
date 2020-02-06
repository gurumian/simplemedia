'use strict'

const {Source, AudioDecoder, AudioRenderer, VideoRenderer, VideoDecoder} = require('bindings')('simplemedia');
const adjust = 0;
module.exports = class MediaPlayer {
  constructor(renderer) {
    this.source = new Source();
    this.audio = {};
    this.video = {};
    this.renderer = renderer; // native renderer
    
    this.audio.renderer = new AudioRenderer();
    if(renderer) {
      this.video.renderer = new VideoRenderer(renderer);
    }
    
    this.count=0;

    this.source.trace = true;
    this.audio.renderer.trace = true;

    this.isFirstFrame = true;
  }

  _prepareAudio(fmt, source) {
    let pid = source.audioPid;
    console.log('pid: ' + pid);
    let pidchannel = source.requestPidChannel({
      pid : pid,
    });

    this.audio.decoder = new AudioDecoder();
    this.audio.decoder.prepare(fmt.streams[pid]['native']);
    this.audio.decoder.pidchannel = pidchannel;
    this.audio.decoder.trace = true;

    this.audio.renderer.prepare({
      samplerate: this.audio.decoder.samplerate,
      channels: this.audio.decoder.channels,
      channellayout: this.audio.decoder.channellayout,
      sampleformat: this.audio.decoder.sampleformat,
    });
  }

  _prepareVideo(fmt, source) {
    let pid = source.videoPid;
    console.log('pid: ' + pid);
    let pidchannel = source.requestPidChannel({
      pid : pid,
    });

    this.video.decoder = new VideoDecoder();
    this.video.decoder.prepare(fmt.streams[pid]['native']);
    this.video.decoder.pidchannel = pidchannel;
    this.video.decoder.trace = true;

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
        if(this.isFirstFrame) {
          this.audio.pts = frame.pts;
          this.isFirstFrame = false;
        }
        else {
          delay = frame.pts - this.audio.pts;
          this.audio.pts = frame.pts;
        }

        this.audio.renderer.render(frame.data);
        this.count++;
       
        delay = Math.floor(delay / 1000) - adjust;
        setTimeout(()=>{
          this._decodeAudio();
        }, delay);
      }
      else {
        console.log('null packet');
        console.log('count: ' + this.count);
        setTimeout(()=>{
          if(this.onend)
            this.onend();
        });
      }
    });
  }

  _decodeVideo() {
    this.video.decoder.decode(frame => {
      var delay = 0;
      if(frame) {
        if(this.isFirstFrame) {
          this.video.pts = frame.pts;
          this.isFirstFrame = false;
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

        this.video.renderer.render(frame.data);
        this.count++;
       
        delay = Math.floor(delay / 1000) - adjust;
        setTimeout(()=>{
          this._decodeVideo();
        }, delay);
      }
      else {
        console.log('null packet');
        console.log('count: ' + this.count);
      }
    });
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
    }, 100);

    setTimeout(()=>{
      this._decodeVideo();
    }, 100);
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
}