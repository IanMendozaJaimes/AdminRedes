const dns = require('dns');
const readline = require('readline');

const rl = readline.createInterface({
    input: process.stdin,
    output: process.stdout
});

rl.question('Nombre del dominio: ', (domainName) => {
    dns.resolve4(domainName, function (err, addresses) {
        if(addresses){
            console.log('Registros A');
            console.log(addresses);
        }
    });    
    dns.resolveNs(domainName, function (err, addresses) {
        if(addresses){
            console.log('Registros NS: ');
            console.log(addresses);
        }
    });
    dns.resolve6(domainName, function (err, addresses) {
        if(addresses){
            console.log('Registros AAAA: ');
            console.log(addresses);
        }
    });
    dns.resolveCname(domainName, function (err, addresses) {
        if(addresses){
            console.log('Registros CNAME: ');
            console.log(addresses);
        }
    });
    dns.resolveMx(domainName, function (err, addresses) {
        if(addresses){
            console.log('Registros MX')
            console.log(addresses);
        }
    });
    dns.resolveNaptr(domainName, function (err, addresses) {
        if(addresses){
            console.log('Registros resolveNaptr: ');
            console.log(addresses);
        }
    });

    rl.close();
});



  
