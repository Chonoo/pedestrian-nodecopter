var sleep = require('sleep')
var fs = require('fs');
var Parrot = require('./build/Release/objects_detection_addon')


var obj = new Parrot.Objects_Detection_Wrapper();
var png_raw_data = fs.readFileSync('IZIEbrv.png')
sleep.sleep(2)
console.log("STARTING COMPUTATIONS")
a = obj.compute(png_raw_data)
console.log("ENDING COMPUTATIONS")
var png_raw_data = fs.readFileSync('image_00000005_1.png')
sleep.sleep(2)
a = obj.compute(png_raw_data)
console.log("DONE")
console.log(a)
/*try {
        console.log(a.1.pmx);
        console.log(a.1.pmy);
        console.log(a.1.pxx);
        console.log(a.1.pxy); }
catch (Error) {
    console.log("Property failed") }*/
sleep.sleep(5)

