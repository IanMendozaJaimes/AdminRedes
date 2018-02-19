var dnsd = require('dnsd')
dnsd.createServer(function(req, res) {
  res.end('127.0.0.1')
}).listen(53, '127.0.0.1')

console.log('Server running at 127.0.0.1:53')