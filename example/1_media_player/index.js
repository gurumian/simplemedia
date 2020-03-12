'use strict'

var args = process.argv.slice(2);
console.log('args: ', args);


const {Window, MediaPlayer, State} = require('simplemedia');

var uri = 'https://sample-videos.com/video123/mp4/720/big_buck_bunny_720p_1mb.mp4'
if(args.length) {
  uri = args[0];
}

const Event = Object.freeze({
  'quit':256,
  'keyPressed':768,
  'keyReleased':769,
  'VK_RIGHT':1073741903,
  'VK_LEFT':1073741904,
  'VK_DOWN':1073741905,
  'VK_UP': 1073741906,
  'VK_SPACE':32,
  'VK_ENTER':13,
});

function onkeypressed(key) {
  switch(key) {
    case Event.VK_RIGHT: {
      var pos = player.position;
      pos += 1000000;
      if(pos < player.duration)
        player.position = pos;
      break;
    }
    case Event.VK_LEFT: {
      player.position -= 1000000;
      break;
    }
    case Event.VK_DOWN: {
      player.volume -= 0.1;
      break;
    }
    case Event.VK_UP: {
      player.volume += 0.1;
      break;
    }
    case Event.VK_SPACE: {
      if(player.state == State.started) {
        player.pause();
      }
      else {
        player.resume();
      }
      break;
    }
    case Event.VK_ENTER: {
      window.fullscreen = !window.fullscreen;
      break;
    }
  }
}

function readAndDispatch() {
  let event = window.pollEvent();
  if(event) {
    // console.log(event);
    switch(event.type) {
    case Event.quit:
      process.exit();
      break;

    case Event.keyPressed:
      onkeypressed(event.key);
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

let player = new MediaPlayer({
  renderer: renderer,
  trace: true,
});
player.datasource = uri;
player.prepare().then(resolve => {
  console.log(resolve);
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