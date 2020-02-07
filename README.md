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