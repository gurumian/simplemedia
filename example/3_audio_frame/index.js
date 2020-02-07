'use strict'

var args = process.argv.slice(2);
console.log('args: ', args);

const {Source, AudioDecoder} = require('simplemedia');

var media_uri='https://file-examples.com/wp-content/uploads/2017/11/file_example_MP3_700KB.mp3';
if(args.length) {
  media_uri = args[0];
}

var count = 0;
var decoder = null;
var channels = 0;

function dump(frame) {
  let data = frame.data;
  let samples = frame.nb_samples;

  for(var i = 0; i < samples; i++) {
    let sample = data[i];
    for(var j = 0; j < channels; j++) {
      console.log(sample[j]);
    }
  }
}

function decode() {
  decoder.decode(frame => {
  if(frame) {
    dump(frame);
    setTimeout(decode);
  }
  else {
    console.log('null packet');
    console.log('count: ' + count);

    decoder = null;
    source = null;
  }
  });	
}

var source = new Source();
source.datasource = media_uri;
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
let pidchannel = source.requestPidChannel({
  pid : pid,
});

decoder = new AudioDecoder();
decoder.prepare(fmt.streams[pid]['native']);
decoder.pidchannel = pidchannel;
channels = fmt.streams[pid]['channels'];

source.start();
setTimeout(decode);

console.log('Press any key to exit');
process.stdin.setRawMode(true);
process.stdin.resume();
process.stdin.on('data', process.exit.bind(process, 0));