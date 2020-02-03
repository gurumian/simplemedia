'use strict'

const {Source, AudioDecoder, AudioRenderer} = require('bindings')('simplemedia');

module.exports = class AudioPlayer {
  constructor() {
    this.source = new Source();
    this.renderer = new AudioRenderer();
    this.count=0;
  }

  async prepare() {
    let source = this.source;
    return new Promise(resolve => {
      source.prepare(args => {
        if(source.hasAudio) {
          let pid = source.audioPid;
          console.log('pid: ' + pid);
          let pidchannel = source.requestPidChannel({
            pid : pid,
          });
      
          this.decoder = new AudioDecoder();
          this.decoder.prepare(source.findStream(pid));
          this.decoder.pidchannel = pidchannel;
      
          this.renderer.prepare({
            samplerate: this.decoder.samplerate,
            channels: this.decoder.channels,
            channellayout: this.decoder.channellayout,
            sampleformat: this.decoder.sampleformat,
          });
        }
        resolve();
      });
    });  
  }

  _decode() {
    this.decoder.decode(frame => {
      if(frame) {
        this.renderer.render(frame);
        this.count++;
       
        setTimeout(()=>{
          this._decode();
        }, 0 );
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
    this._decode();
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


