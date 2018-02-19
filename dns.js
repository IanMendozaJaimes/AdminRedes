var dnsd = require('dnsd')
let listaNegra=["xvideos.com,redtube.com,pornhub.com"]


dnsd.createServer(function(req, res) {
    console.log(req.question)
    let domainName=req.question[0].name
    if(listaNegra.indexOf(domainName) >= 0){
        res.end('127.0.0.1')
    }else{
        /*dns.resolve4(domainName, function (err, addresses) {
            if(addresses){
                console.log('Registros A');
                console.log(addresses[0]);
            }
        })*/
        res.end('127.0.0.1')
    }
}).listen(53, '127.0.0.1')

console.log('Server running at 127.0.0.1:53')