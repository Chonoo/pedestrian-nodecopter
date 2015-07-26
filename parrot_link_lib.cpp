#include <node.h>
#include <v8.h>
#include "objects_detection_lib.hpp"



using namespace v8;


objects_detection::detections_t
/* // One of these
#if defined(MONOCULAR_OBJECTS_DETECTION_LIB)
void init_objects_detection(const boost::program_options::variables_map options,
                            boost::shared_ptr<doppia::CameraCalibration> calibration_p,
                            const bool use_ground_plane = false);
#else
/// this initialization function does not use the video_input.calibration_filename option
/// the camera calibration is given directly
void init_objects_detection(const boost::program_options::variables_map options,
                            boost::shared_ptr<doppia::StereoCameraCalibration> stereo_calibration_p,
                            const bool use_ground_plane = false, const bool use_stixels = false);
*/

// We want the rest of the module to run independently
/// non-blocking call to launch the detections, pool detections_are_ready to check for new results
//void compute_async();

// objects_detection::compute_async()



// Receiving a picture from getPngStream in Node


// Upon receiving a picture from getPngStream in Node
/* if ( objects_detection::detections_are_ready() ) {



}

*/










void Method(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);
  args.GetReturnValue().Set(String::NewFromUtf8(isolate, "world"));
}

void init(Handle<Object> exports) {
  NODE_SET_METHOD(exports, "parrot_link_lib", Method);
}

NODE_MODULE(addon, init)
