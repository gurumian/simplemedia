'use strict';

const {Source, AudioDecoder, AudioRenderer, VideoRenderer, VideoDecoder, Window} = require('bindings')('simplemedia');
import {MediaPlayer} from './lib/media_player';
import {Timer} from './lib/timer';
export {Source,AudioDecoder,AudioRenderer,VideoRenderer,VideoDecoder,Window,MediaPlayer,Timer};