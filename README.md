# utun

Create a utun interface from node

```
npm install utun
```

## Usage

``` js
var utun = require('utun')
var ip = require('ip-packet')

var tunnel = utun()

tunnel.on('message', function (message) {
  // message is a raw IP packet
  console.log(ip.decode(message))
  // lets just echo it back so ping works
  tunnel.send(message)
})
```

If you run the above code (as root) you should be able
to see a new `utun0` device in `ifconfig` and ping the device
by doing `ping 10.114.0.49`

This has currently only been tested on OSX Yosemite. YMMV.

## License

MIT
