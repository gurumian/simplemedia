## Prerequisites
```bash
apt install libavformat-dev libavcodec-dev libavutil-dev libswscale-dev libsdl2-dev

(or brew)
```

## Install
```bash
npm i simplemedia --save
```

## Example
See `example/`

```js
const {Window, MediaPlayer} = require('simplemedia');


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
```


## (Optional) Native only build
```bash
cd native;
mkdir build; cd buiild
cmake .. && make
```
You'll probably get a binary named `mediaplayer`
Simply, try to run it with a media file
```bash
./mediaplayer /path/to/file
```