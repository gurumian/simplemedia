const {Window, Source, AudioRenderer, AudioDecoder, VideoDecoder} = require('..')

var chai = require('chai')
var assert = chai.assert    // Using Assert style
var expect = chai.expect    // Using Expect style
var should = chai.should()  // Using Should style

const fs = require('fs')

const uri = './test/test.mp4'

describe('All', function() {
  if (! fs.existsSync(uri)) {
    console.log(`No such file: ${uri}. Please download the file in a bash console`)
    console.log('e.g) wget -O ./test/test.mp4 https://sample-videos.com/video123/mp4/720/big_buck_bunny_720p_1mb.mp4')
    return
  }

  let source = new Source()
  let fmt = null
  let audioPidchannel = null
  let audioDecoder = null
  let audioRenderer = null

  describe('Source', function() {
    it('should return -1 when the value is not present', function() {
      source.datasource = uri
      assert.equal(source.datasource, uri)
    })

    it('should return -1 when the value is not present', function() {
      fmt = source.prepare()
      assert.notEqual(fmt, null)
      assert.equal(typeof fmt, 'object')
      assert.equal(fmt.hasOwnProperty('format'), true)
      assert.equal(fmt.hasOwnProperty('streams'), true)
      assert.equal(source.hasAudio, true)
      assert.equal(source.hasVideo, true)
      assert.equal(typeof source.duration, 'number')
      assert.equal(typeof source.videoPid, 'number')
      assert.equal(typeof source.audioPid, 'number')
    })

    it('should return -1 when the value is not present', function() {
      let pidchannel = source.requestPidChannel(source.audioPid)
      assert.notEqual(pidchannel, null)
      audioPidchannel = pidchannel
    })
  })

  audioDecoder = new AudioDecoder()
  describe('AudioDecoder', function() {
    it('should return -1 when the value is not present', function() {
      assert.notEqual(fmt.streams[source.audioPid].native, null)
      let strm = fmt.streams[source.audioPid].native
      audioDecoder.prepare(strm)
      audioDecoder.pidchannel = audioPidchannel
    })
  })

  audioRenderer = new AudioRenderer()
  describe('AudioRenderer', function() {
    it('should return -1 when the value is not present', function() {
      audioRenderer.prepare({
        samplerate: audioDecoder.samplerate,
        channels: audioDecoder.channels,
        channellayout: audioDecoder.channellayout,
        sampleformat: audioDecoder.sampleformat,
      });
    })

    it('should return -1 when the value is not present', function() {
      source.start()
      audioDecoder.decode()
      .then(frame => {
        // assert.notEqual(frame, null)
      })
    })
  })
})