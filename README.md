![Node.js CI](https://github.com/gurumian/simplemedia/workflows/Node.js%20CI/badge.svg)
[![FOSSA Status](https://app.fossa.io/api/projects/git%2Bgithub.com%2Fgurumian%2Fsimplemedia.svg?type=shield)](https://app.fossa.io/projects/git%2Bgithub.com%2Fgurumian%2Fsimplemedia?ref=badge_shield)


## Prerequisites
```bash
apt install libavformat-dev libavcodec-dev libavutil-dev libswscale-dev libsdl2-dev

(or brew)
```

You may need to install `cmake-js` and `typescript`
```
npm i -g cmake-js
npm i -g typescript
```


## Install
```bash
npm i simplemedia
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
mkdir build; cd build
cmake .. && make
```
You'll probably get a binary named `mediaplayer`
try to run it with a media file
```bash
./mediaplayer /path/to/file
```


## License
[![FOSSA Status](https://app.fossa.io/api/projects/git%2Bgithub.com%2Fgurumian%2Fsimplemedia.svg?type=large)](https://app.fossa.io/projects/git%2Bgithub.com%2Fgurumian%2Fsimplemedia?ref=badge_large)
