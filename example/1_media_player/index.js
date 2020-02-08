'use strict'

var args = process.argv.slice(2);
console.log('args: ', args);


const {Window, MediaPlayer} = require('simplemedia');

var media_uri = 'https://file-examples.com/wp-content/uploads/2017/04/file_example_MP4_480_1_5MG.mp4'
if(args.length) {
  media_uri = args[0];
}

function readAndDispatch() {
  let event = window.pollEvent();
  if(event) {
    console.log(event);
    switch(event.type) {
    case 768:
      switch(event.key) {
      case 1073741903: { // right
        player.position += 1000000;
        break;
      }
      case 1073741904: { // left
        player.position -= 1000000;
        break;
      }
      case 1073741905: { // down
        player.volume -= 0.1;
        break;
      }
      case 1073741906: { // up
        player.volume += 0.1;
        break;
      }
      case 32: { // space
        // TODO: pause & resume
        break;
      }
      case 13: { // space
        // TODO: full screen
        break;
      }
      }
    }
  }
  setTimeout(readAndDispatch);
}

let window = new Window({
  title: "simplemedia nodejs",
  width: 640,
  height: 480,
});
let renderer = window.createRenderer();

let player = new MediaPlayer(renderer);
player.datasource = media_uri;
player.prepare().then(resolve => {
  console.log('duration: ' + player.duration);
  player.start();
}).catch(err => {
  console.log(err);
});

player.onend = (() => {
  console.log('end-of-stream!');
});

readAndDispatch();

console.log('Press any key to exit');
process.stdin.setRawMode(true);
process.stdin.resume();
process.stdin.on('data', process.exit.bind(process, 0));