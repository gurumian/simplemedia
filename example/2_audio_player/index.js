'use strict'

var args = process.argv.slice(2);
console.log('args: ', args);

const {MediaPlayer} = require('simplemedia');

var uri='https://file-examples.com/wp-content/uploads/2017/11/file_example_MP3_700KB.mp3';
if(args.length) {
  uri = args[0];
}

let player = new MediaPlayer({
  trace: true,
});
player.datasource = uri;
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