'use strict'

const {Source, AudioDecoder, AudioRenderer} = require('bindings')('simplemedia');
const adjust = 3;
module.exports = class AudioPlayer {
  constructor() {
    this.source = new Source();
    this.renderer = new AudioRenderer();
    this.count=0;

    this.source.trace = true;
    this.renderer.trace = true;

    this.isFirstFrame = true;
    this.pts = 0;
  }

  async prepare() {
    let source = this.source;
    return new Promise((resolve, reject) => {
      let fmt = source.prepare();
      if(fmt) {
        console.log(fmt);
        if(source.hasAudio) {
          let pid = source.audioPid;
          console.log('pid: ' + pid);
          let pidchannel = source.requestPidChannel({
            pid : pid,
          });
      
          this.decoder = new AudioDecoder();
          this.decoder.prepare(fmt.streams[pid]['native']);
          this.decoder.pidchannel = pidchannel;
          this.decoder.trace = true;
      
          this.renderer.prepare({
            samplerate: this.decoder.samplerate,
            channels: this.decoder.channels,
            channellayout: this.decoder.channellayout,
            sampleformat: this.decoder.sampleformat,
          });
        }
        resolve(fmt);
      }
      else {
        reject();
      }
    });  
  }

  _decode() {
    this.decoder.decode(frame => {
      var delay = 0;
      if(frame) {
        if(this.isFirstFrame) {
          this.pts = frame.pts;
          this.isFirstFrame = false;
        }
        else {
          delay = frame.pts - this.pts;
          this.pts = frame.pts;
        }

        this.renderer.render(frame.data);
        this.count++;
       
        setTimeout(()=>{
          this._decode();
        }, (delay / 1000) - adjust);
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

  /**
   * @param {() => void} onend
   */
  set onended(onend) {
    this.onend = onend;
  }

  start() {
    this.source.start();
    setTimeout(()=>{
      this._decode();
    });
  }

  stop() {
    this.decoder.stop();
    this.source.stop();
  }

  pause() {
    this.source.pause();
    this.decoder.pause();
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


