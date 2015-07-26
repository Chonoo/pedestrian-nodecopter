#include <node.h>

#include <node_object_wrap.h>
#include "objects_detection_lib.hpp"
#include "objects_detection_wrapper.h"

// This is a wrapped Node objects, as documented in 
// https://nodejs.org/api/addons.html
// Functions and variabled prefixed with v8 are node objects
// and follow a specific API structure.

// Note: v8 functions don't have return values. They pass info around
// as object properties
using namespace v8;


extern "C" {
void InitAll(Handle<Object> exports) {
    Objects_Detection_Wrapper::Init(exports);
}

NODE_MODULE(objects_detection_addon, InitAll)
}
