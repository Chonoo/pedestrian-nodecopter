var drone = require('ar-drone')
var client = drone.createClient();
var Parrot = require('./build/Release/objects_detection_addon')
var obj = new Parrot.Objects_Detection_Wrapper();


var http = require('http');

/*function main(client, opts) {
  var png = null;
    console.log("ALIVE")
  opts = opts || {};
  
  var server = http.createServer(function(req, res) {

    if (!png) {
      png = client.getPngStream();
      png.on('error', function (err) {
          console.error('png stream ERROR: ' + err);
      });
    }
    console.log("EN")
    res.writeHead(200, { 'Content-Type': 'multipart/x-mixed-replace; boundary=--daboundary' });

    png.on('data', sendPng);

    function sendPng(buffer) {
      detection = obj.compute(buffer)

        //for (var i = -15; i < 15; i++) {
        //    buffer[(detection.pmx+i)*detection.pmy] = 0
        //    buffer[detection.pmx*(i*detection.pmy)] = 0
        //}
        
      console.log(buffer.length);
      res.write('--daboundary\nContent-Type: image/png\nContent-length: ' + buffer.length + '\n\n');
      res.write(buffer);
    }
  });

  server.listen(opts.port || 8000);
};


main(client)
*/




var pngStream = client.getPngStream();
count = 0
function wrap(ind) {a = obj.compute(ind); 
console.log("POSITIONS", count)
    for (var i = 0; i < a.length; i++){ 
        console.log(a[i].tL.x);
    }


}


pngStream.on('data', wrap); 
