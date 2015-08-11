var mutexify = require('mutexify')
var utun = require('./build/Release/utun')
var fs = require('fs')
var proc = require('child_process')
var events = require('events')

var noop = function () {}

module.exports = function (opts) {
  if (!opts) opts = {}
  if (!opts.netmask) opts.netmask = '255.255.255.255'
  if (!opts.inet) opts.inet = '10.114.0.49'

  var that = new events.EventEmitter()
  var lock = mutexify()

  var receive = new Buffer(4096)
  var send = new Buffer(4096)

  var fd = utun.utun(onchange)

  send[0] = 0
  send[1] = 0
  send[2] = 0
  send[3] = 2 // yolo

  // TODO: syscall this? and do not hardcode the interface
  proc.exec('ifconfig utun0 inet ' + opts.inet + ' ' + opts.inet + ' netmask ' + opts.netmask, function (err) {
    if (err) that.emit('error', err)
    else that.emit('ready')
  })

  function onchange () {
    lock(function (release) {
      fs.read(fd, receive, 0, receive.length, null, function (err, n) {
        if (err) return release()
        that.emit('message', receive.slice(4, n))
        release()
      })
    })
  }

  that.send = function (buf, cb) {
    buf.copy(send, 4)
    fs.write(fd, send, 0, buf.length + 4, null, cb || noop)
  }

  return that
}
