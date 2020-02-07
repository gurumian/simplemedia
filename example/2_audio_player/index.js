'use strict'

const AudioPlayer = require('./audio_player');

const test_media_uri='https://file-examples.com/wp-content/uploads/2017/11/file_example_MP3_700KB.mp3';

let player = new AudioPlayer();
player.datasource = test_media_uri;
player.prepare().then(resolve => {
  console.log('prepared');
  player.start();
}).catch(err => {
  console.log(err);
});

player.onend = (() => {
  console.log('end-of-stream!');
});

console.log('Press any key to exit');
process.stdin.setRawMode(true);
process.stdin.resume();
process.stdin.on('data', process.exit.bind(process, 0));