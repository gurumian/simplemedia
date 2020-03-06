'use strict';

module.exports = require('bindings')('simplemedia')
module.exports.MediaPlayer = require('./lib/media_player')
module.exports.Timer = require('./lib/timer')