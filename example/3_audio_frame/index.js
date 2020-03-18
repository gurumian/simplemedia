'use strict'

var args = process.argv.slice(2);
console.log('args: ', args);

const {Source, AudioDecoder, Timer} = require('simplemedia');

var uri='https://file-examples.com/wp-content/uploads/2017/11/file_example_MP3_700KB.mp3';
if(args.length) {
  uri = args[0]
}

var count = 0
var decoder = null
var channels = 0

function dump(frame) {
  let data = frame.data
  console.log(data)
}

function decode() {
  decoder.decode()
  .then(frames => {
    if(frames) {
      frames.map((frame) => {
        count++;
        dump(frame);
      })
      setTimeout(decode)
    }
    else { // eos
      console.log('null packet');
      console.log(`frame count: ${count} ${timer.dt}(ms)`);
      decoder = null;
      source = null;
    }
  })
}

let timer = new Timer();
var source = new Source();
source.datasource = uri;
let fmt = source.prepare();
if(!fmt) {
  console.log('failed to prepare the source');
  process.exit();	
}

console.log(fmt);

if(! source.hasAudio) {
  console.log('failed to prepare the source');
  process.exit();	
}

let pid = source.audioPid;
console.log('pid: ' + pid);
let pidchannel = source.requestPidChannel(pid);

decoder = new AudioDecoder();
decoder.prepare(fmt.streams[pid]);
decoder.pidchannel = pidchannel;
channels = fmt.streams[pid]['channels'];

timer.update();
source.start();
setTimeout(decode);

console.log('Press any key to exit');
process.stdin.setRawMode(true);
process.stdin.resume();
process.stdin.on('data', process.exit.bind(process, 0));