var utun = require('./')
var ip = require('ip-packet')

var tunnel = utun()

tunnel.on('message', function (message) {
  console.log(ip.decode(message))
  tunnel.send(message)
})