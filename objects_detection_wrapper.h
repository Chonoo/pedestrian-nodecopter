#ifndef OBJECTS_DETECTION_WRAPPER_HPP
#define OBJECTS_DETECTION_WRAPPER_HPP

#include <node.h>
#include <node_object_wrap.h>
#include <node_buffer.h>
#include "objects_detection_lib.hpp"

// typedefs
#include <vector>
#include "objects_detection/Detection2d.hpp"

// We need ... stuff from boost

#define png_infopp_NULL (png_infopp)NULL
#define int_p_NULL (int*)NULL
#include <boost/gil/typedefs.hpp>
#include "video_input/calibration/CameraCalibration.hpp"
#include <boost/gil/image.hpp>
#include <boost/gil/image_view.hpp>
#include <boost/gil/extension/io/png_io_private.hpp>
//#include <boost/gil/extension/io/png_io.hpp>
//#include <boost/gil/extension/dynamic_image/any_image.hpp>

//#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
//#include <boost/scoped_ptr.hpp>
//#include <boost/shared_ptr.hpp>
//#include <boost/format.hpp>
//#include <boost/foreach.hpp>
//#include <boost/thread.hpp> 

// Reading Detection response
#include <boost/cstdint.hpp>
#include <boost/lexical_cast.hpp>

// Reading and validating PNGs
//#include <png.h>
//#include <iomanip>

#include <iostream>
//#include <stdio.h>

#include <string>

// This is a wrapped Node objects, as documented in 
// https://nodejs.org/api/addons.html
// Functions and variabled prefixed with v8 are node objects
// and follow a specific API structure.

// Note: v8 functions don't have return values. They pass info around
// as object properties

bool is_first_call = true; // We only want to initiate one objects_detection
// Since objects_detection_lib lives in global for now

// These typedefs are the same as in objects_detection_lib //
//typedef doppia::Detection2d detection_t;
//typedef std::vector<detection_t> detections_t;

void first_call_setup () {
        const bool use_ground_plane = false; // for clarity
        const bool use_stixels = false; // for clarity
        //std::string configuration_filepath =  "./parrot.config.ini"; // How does this work? 
        boost::filesystem::path configuration_filepath =  "./parrot.config.ini"; // How does this work? 
        objects_detection::init_objects_detection(configuration_filepath, use_ground_plane, use_stixels); // The first option in objects_detection_lib 
        is_first_call = false;


}


class Objects_Detection_Wrapper: public node::ObjectWrap {
    public:
        static void Init(v8::Handle<v8::Object> exports); // v8 function
    private:
        explicit Objects_Detection_Wrapper(double value = 0); // No type conversion (v8 thing)
        ~Objects_Detection_Wrapper(); // Nothing dynamically assigned yet

        static void New(const v8::FunctionCallbackInfo<v8::Value>& args); // v8 function
        static void Compute(const v8::FunctionCallbackInfo<v8::Value>& args); // Accepts JS Buffer
        static v8::Persistent<v8::Function> constructor; // v8 function

        double value_;
};

// TODO: fix compiler
// For some reason neither node-gyp or cmake-js include 
// the definitions in objects_detection_wrapper.cpp (or .cc) 
// so object specifications have to happen below

using namespace v8; // TODO: Figure out which example functions are v8
// and reinstitute this namespace

Persistent<Function> Objects_Detection_Wrapper::constructor;

// Houskeeping functions
Objects_Detection_Wrapper::Objects_Detection_Wrapper(double value) : value_(value) {
}


Objects_Detection_Wrapper::~Objects_Detection_Wrapper() {}


void Objects_Detection_Wrapper::Init(Handle<Object> exports) {
    Isolate* isolate = Isolate::GetCurrent();

    // Prepare constructor template
    Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
    tpl->SetClassName(String::NewFromUtf8(isolate, "Objects_Detection_Wrapper"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1); //TODO: Check 1 or 2? // Seems to be 1

    // Prototype(s)
    NODE_SET_PROTOTYPE_METHOD(tpl, "compute", Compute); // Main Computing Function
    
    // OD Code
    bool use_ground_plane = false, use_stixels = false;


    constructor.Reset(isolate, tpl->GetFunction());
    exports->Set(String::NewFromUtf8(isolate, "Objects_Detection_Wrapper"),
            tpl->GetFunction());
}


void Objects_Detection_Wrapper::New(const FunctionCallbackInfo<Value>& args) {
    if (is_first_call) { // Declared at the top of the file
        first_call_setup();
    } // End of objects_detection init

    Isolate* isolate = Isolate::GetCurrent();

    HandleScope scope(isolate);
    
    if (args.IsConstructCall()) {
        // Invoked as constructor: 'new Objects_Detection_Wrapper(...)'
        double value = args[0]->IsUndefined() ? 0 : args[0]->NumberValue(); // uncommented for testing
        // MyObject obj = new MyObject(value); // Last two for reference
        Objects_Detection_Wrapper* obj = new Objects_Detection_Wrapper(value);
        obj->Wrap(args.This());
        args.GetReturnValue().Set(args.This());
    } else {
        // Invoked as plain function 'Objects_Detection_Wrapper(...)', turn into construction call.
        const int argc = 1;
        Local<Value> argv[argc] = { args[0] };
        Local<Function> cons = Local<Function>::New(isolate, constructor);
        args.GetReturnValue().Set(cons->NewInstance(argc, argv));
    }
    

}


void Objects_Detection_Wrapper::Compute(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    // This is a lot safer than it looks
    unsigned char* buf = reinterpret_cast<unsigned char*>(node::Buffer::Data(args[0]));//.As<v8::Object>());
    size_t buf_length = node::Buffer::Length(args[0]); // Provides us with array length
    // We know that node:Buffer gives us a char* array
    // And for hex values we need an unsigned char* array
    // So at this stage everything looks like an &istream 
    
    FILE* boost_input = tmpfile();
    for (int i = 0; i < buf_length; i++) {
        fputc(buf[i], boost_input); // put integer in stream
    }
    rewind(boost_input);

    // png_io_private.hpp
    boost::gil::detail::png_reader_color_convert<boost::gil::default_color_converter> m(boost_input, boost::gil::default_color_converter());

    boost::gil::rgb8_image_t im; //TODO: Make this one step (no intermediary image)
    boost::gil::rgb8c_view_t doppia_input_image;
    m.read_image(im);
    doppia_input_image = boost::gil::view(im);

    objects_detection::set_monocular_image(doppia_input_image);
    objects_detection::compute(); // blocking call
    //std::cout << "Survived Computations";
    objects_detection::detections_t detections = objects_detection::get_detections();
    // TODO: Implement safe call: (objects_detection::detections_are_ready() ? objects_detection::get_detections() : [empty detections vector] );

    // TODO: Unpack vector and return it

    boost::int16_t p_x;
    boost::int16_t p_y;

    doppia::geometry::point_xy<boost::int16_t> point_min;
    doppia::geometry::point_xy<boost::int16_t> point_max;
    v8::Local<v8::Object> result;

    //int det_size = detections.size();
    v8::Handle<v8::Array> results = v8::Array::New( isolate, detections.size() );
    for (size_t i = 0; i < detections.size(); i++) {
        result = v8::Object::New(isolate);
        point_min =  detections[i].bounding_box.min_corner();
        point_max =  detections[i].bounding_box.max_corner(); 

        p_x = point_min.x();
        p_y = point_min.y();
        v8::Local<v8::Object> top_left = v8::Object::New(isolate);
        top_left->Set(String::NewFromUtf8(isolate, "x"), Integer::New(isolate, p_x));
        top_left->Set(String::NewFromUtf8(isolate, "y"), Integer::New(isolate, p_y));
        //char xc = boost::lexical_cast<char, int16_t>(p_x);
        //result->Set(String::NewFromUtf8(isolate, "pmx"), Integer::New(isolate, p_x));
        //result->Set(String::NewFromUtf8(isolate, "pmy"), Integer::New(isolate, p_y));

        p_x = point_max.x();
        p_y = point_max.y();
        v8::Local<v8::Object> bottom_right = v8::Object::New(isolate);
        bottom_right->Set(String::NewFromUtf8(isolate, "x"), Integer::New(isolate, p_x));
        bottom_right->Set(String::NewFromUtf8(isolate, "y"), Integer::New(isolate, p_y));

        result->Set(String::NewFromUtf8(isolate, "tL"), top_left);
        result->Set(String::NewFromUtf8(isolate, "bR"), bottom_right);

        //result->Set(String::NewFromUtf8(isolate, "pxx"), Integer::New(isolate, p_x));
        //result->Set(String::NewFromUtf8(isolate, "pxy"), Integer::New(isolate, p_y));
        results->Set(i, result);
    }

    
    //obj->Set(String::NewFromUtf8(isolate, "msg"), args[0]->ToString());



    args.GetReturnValue().Set(results);//Number::New(isolate, true));
}



























































// We are just going to see if this compiles... /// I'm hungry, fix when get home

/*
program_options::options_description TestObjectsDetectionApplication::get_args_options()
{
    program_options::options_description desc("TestObjectsDetectionApplication options");
    desc.add_options()

            ("configuration_file,c",
         #if defined(MONOCULAR_OBJECTS_DETECTION_LIB)
             program_options::value<string>()->default_value("test_monocular_objects_detection_lib.config.ini"),
         #else
             program_options::value<string>()->default_value("test_objects_detection_lib.config.ini"),
         #endif
             "indicates the path of the configuration .ini file")

            ("save_detections",
             program_options::value<bool>()->default_value(false),
             "save the detected objects in a data sequence file (only available in monocular mode)")

            // (added directly in the objects_detection::get_options_description, so that it can be included in the config file)
            //("video_input.images_folder,i", program_options::value<string>(),
            // "path to a directory with monocular images. This option will overwrite left/right_filename_mask values")

            ;


    return desc;
}

program_options::variables_map TestObjectsDetectionApplication::parse_arguments(int argc, char *argv[])
{

    program_options::options_description desc("Allowed options");
    desc.add_options()("help", "produces this help message");

    desc.add(TestObjectsDetectionApplication::get_args_options());

    objects_detection::get_options_description(desc);

    program_options::variables_map options;

    try
    {
        program_options::command_line_parser parser(argc, argv);
        parser.options(desc);

        const program_options::parsed_options the_parsed_options( parser.run() );

        program_options::store(the_parsed_options, options);
        //program_options::store(program_options::parse_command_line(argc, argv, desc), options);
        program_options::notify(options);
    }
    catch (std::exception & e)
    {
        cout << "\033[1;31mError parsing the command line options:\033[0m " << e.what () << endl << endl;
        cout << desc << endl;
        exit(EXIT_FAILURE);
    }


    if (options.count("help"))
    {
        cout << desc << endl;
        exit(EXIT_SUCCESS);
    }

    // parse the configuration file
    {

        string configuration_filename;

        if(options.count("configuration_file") > 0)
        {
            configuration_filename = get_option_value<std::string>(options, "configuration_file");
        }
        else
        {
            cout << "No configuration file provided. Using command line options only." << std::endl;
        }

        if (configuration_filename.empty() == false)
        {
            boost::filesystem::path configuration_file_path(configuration_filename);
            if(boost::filesystem::exists(configuration_file_path) == false)
            {
                cout << "\033[1;31mCould not find the configuration file:\033[0m "
                     << configuration_file_path << endl;
                return options;
            }

            printf("Going to parse the configuration file: %s\n", configuration_filename.c_str());

            try
            {
                fstream configuration_file;
                configuration_file.open(configuration_filename.c_str(), fstream::in);
                program_options::store(program_options::parse_config_file(configuration_file, desc), options);
                configuration_file.close();
            }
            catch (...)
            {
                cout << "\033[1;31mError parsing THE configuration file named:\033[0m "
                     << configuration_filename << endl;
                cout << desc << endl;
                throw;
            }

            cout << "Parsed the configuration file " << configuration_filename << std::endl;
        }
    }

    return options;
}


// var a = require("./objects_detection_addon")
*/


#endif
