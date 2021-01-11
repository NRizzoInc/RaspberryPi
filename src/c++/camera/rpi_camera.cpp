#include "rpi_camera.h"

namespace RPI {

namespace Camera {

// for convenience
using std::cout;
using std::cerr;
using std::endl;

// these paths/files comes from the opencv package
// const fs::path classifiers_dir          {"/usr/share/opencv/haarcascades/"};
const fs::path CURR_DIR                 {fs::path{__FILE__}.parent_path()};
const fs::path classifiers_dir          {CURR_DIR / "classifiers"};

/********************************************** Constructors **********************************************/

CamHandler::CamHandler(
    const bool verbosity,
    const int max_frame_count,
    const bool should_init,
    const std::string face_xml,
    const std::string eye_xml
)
    : raspicam::RaspiCam_Cv{}
    , is_verbose{verbosity}
    , frame_count{0}
    , max_frames{max_frame_count}       // defaults to infinite = -1
    , stop_thread{false}
    , should_record{false}
    , facial_classifier{std::pair{
        face_xml != ""  ? fs::path{face_xml} : fs::path{classifiers_dir / "haarcascade_frontalface.xml"},
        cv::CascadeClassifier{}
    }}
    , eye_classifier{std::pair{
        eye_xml != "" ?  fs::path{eye_xml} : fs::path{classifiers_dir / "haarcascade_eye_tree_eyeglasses.xml"},
        cv::CascadeClassifier{}
    }}
{
    if (should_init) {
        if(SetupCam() != ReturnCodes::Success) {
            cerr << "Error: Failed to setup raspicam" << endl;
        }
    }
}

CamHandler::~CamHandler() {
    // stub
}

/********************************************* Getters/Setters *********************************************/

bool CamHandler::getShouldStop() const {
    return stop_thread.load();
}

/**
 * @brief Set whether or not the grabbing loop should end/stop
 * @return ReturnCodes Success if no issues
 */
ReturnCodes CamHandler::setShouldStop(const bool new_status) {
    stop_thread.store(new_status);
    return ReturnCodes::Success;
}

bool CamHandler::getShouldRecord() const {
    return should_record.load();
}

ReturnCodes CamHandler::setShouldRecord(const bool new_status) {
    should_record.store(new_status);
    return ReturnCodes::Success;
}

ReturnCodes CamHandler::setGrabCallback(GrabFrameCb _grab_cb) {
    grab_cb = _grab_cb;
    return ReturnCodes::Success;
}


/********************************************* Camera Functions ********************************************/

ReturnCodes CamHandler::OpenCam() {
    if(!RaspiCam_Cv::open()) {
        cerr << "ERROR: Failed to open raspicam" << endl;
        return ReturnCodes::Error;
    }

    // wait a sec for camera to stabilize
    std::this_thread::sleep_for(std::chrono::seconds(1));

    return ReturnCodes::Success;
}


// note: have to concat strings (not stream it) bc of threading cout overlaps
void CamHandler::RunFrameGrabber(const bool record_immed, const bool should_save) {

    // if should start recording immediately, set accordingly
    if(setShouldRecord(record_immed) != ReturnCodes::Success) {
        cerr << "Failed to set camera recording status" << endl;
    } 

    cout << "Capturing "
         + (max_frames == -1 ? "infinite" : std::to_string(max_frames))
         + " frames" << endl;

    // place to store the grabbed frame
    cv::Mat image;

    // start capture
    // loop until max frame count or told to stop
    start_time      = std::chrono::system_clock::now(); // updates in loop

    cout << "Camera Ready: " + Helpers::Timing::GetTimecode(start_time) + '\n';
    bool was_recording {false};
    while (!getShouldStop() && (max_frames == -1 || frame_count < max_frames)) {

        // do not capture frames unless set to
        if(!getShouldRecord()) {
            // if was recording then need to say we stopped & set new status
            if (was_recording) {
                was_recording = false;
                cout << "Stopping Camera: " + Helpers::Timing::GetTimecode() << endl;
            }
            continue;
        }
        // just started recording (should record + was not previously)
        else if(!was_recording) {
            was_recording = true;

            // update starting time
            start_time = std::chrono::system_clock::now();
            cout << "Starting Camera Capture: " + Helpers::Timing::GetTimecode(start_time) + '\n';
        }

        RaspiCam_Cv::grab();
        RaspiCam_Cv::retrieve(image);

        // make sure valid frame
        if(image.empty()) {
            cerr << "Error: Bad video frame" << endl;
            continue;
        }

        // perform facial recognition (should be done PRIOR to any other modifications)
        if(DetectAndDraw(image) != ReturnCodes::Success) {
            cerr << "Error: Failed to perform facial recogniition on image" << endl;
        }

        // add timestamp to frame (after detection)
        std::string timecode    { Helpers::Timing::GetTimecode() };
        const int fontFace      { cv::FONT_HERSHEY_SIMPLEX };
        const double fontScale  { 1.0 };
        const int thickness     { 2 };
        auto timecode_size      { cv::getTextSize(timecode, fontFace, fontScale, thickness, 0) };
        cv::putText(
            image,                                          // target frames
            timecode,                                       // timecode/stamp text to write
            cv::Point(50, 50),                              // top-left position (otherwise out of screen)
            fontFace,                                       // font style
            fontScale,                                      
            CV_RGB(255, 255, 255),                          // font color (white)
            thickness,
            false                                           // relative to top-left
        );


        // increment frame count
        ++frame_count;

        // use grab callback if provided
        if (grab_cb) {
            // cv::Mat stored as std::vector<uchar (aka unsigned char)> but needed as std::vector<unsigned char>
            std::vector<unsigned char> img_buf;
            cv::imencode(".jpg", image, img_buf);
            grab_cb(img_buf);
        }
    }

    // stop
    RaspiCam_Cv::release();


    if (should_save) {
        constexpr auto filepath {"raspicam_cv_image.jpg"};
        cv::imwrite(filepath, image);
        cout << "Image saved at " + std::string(filepath) << endl;
    }
}

/********************************************* Helper Functions ********************************************/

ReturnCodes CamHandler::SetupCam() {
    // set camera properties & settings
    // TODO: make sure is using h264 raw encoding to mp4
    RaspiCam_Cv::set( CV_CAP_PROP_FORMAT, CV_8UC1 );
    RaspiCam_Cv::set( CV_CAP_PROP_FORMAT, CV_8UC3 );
    RaspiCam_Cv::set( CV_CAP_PROP_FRAME_WIDTH, Constants::Camera::FRAME_WIDTH );
    RaspiCam_Cv::set( CV_CAP_PROP_FRAME_HEIGHT, Constants::Camera::FRAME_HEIGHT );
    RaspiCam_Cv::set( CV_CAP_PROP_FPS, Constants::Camera::VID_FRAMERATE );

    // open camera after setting correct settings
    if(OpenCam() != ReturnCodes::Success) {
        cerr << "Failed to open camera" << endl;
        return ReturnCodes::Error;
    }

    // load facial recognition classifiers
    if (LoadClassifiers() != ReturnCodes::Success) {
        cerr << "Error: Failed to load classifiers" << endl;
        return ReturnCodes::Error;
    }

    return ReturnCodes::Success;
}


/******************************************** Facial Recognition Functions *******************************************/
// credit: https://www.geeksforgeeks.org/opencv-c-program-face-detection/
// credit: https://docs.opencv.org/3.4/db/d28/tutorial_cascade_classifier.html

// how to iterate over cv::Mat:
// -- https://docs.opencv.org/master/d3/d63/classcv_1_1Mat.html#a952ef1a85d70a510240cb645a90efc0d
// pos contains the current pixel's (z,y,z) coordinate that can be used
//      -- pos[0] = x, pos[1] = y, pos[2] = z
// "pixel" is reference to the currently selected pixel in the image at that point
//      -- to modify the contents of the image: pixel.x/y/z = ...


ReturnCodes CamHandler::LoadClassifiers(Classifier& classifiers_pair) {
    const fs::path&         classifier_path     {classifiers_pair.first};
    cv::CascadeClassifier&  classifier_obj      {classifiers_pair.second};
    const bool              load_rtn            {classifier_obj.load(classifier_path.string())};
    return load_rtn ? ReturnCodes::Success : ReturnCodes::Error;
}


ReturnCodes CamHandler::LoadClassifiers() {
    // TODO: use cli to path these files
    const bool face_rtn         {LoadClassifiers(facial_classifier) == ReturnCodes::Success};
    const bool eye_rtn          {LoadClassifiers(eye_classifier) == ReturnCodes::Success};

    // print some debug info
    if (is_verbose) {
        cout << std::boolalpha << "face classifier: " << facial_classifier.first 
             << " (loaded " << !facial_classifier.second.empty() << ")" << endl;

        cout << std::boolalpha << "eye classifier: " << eye_classifier.first
             << " (loaded " << !eye_classifier.second.empty() << ")" << endl;
    }

    // check rtn codes
    if(!face_rtn) {
        cerr << "Error: Failed to load facial recognition classifier: " << facial_classifier.first << endl;
    }

    if (!eye_rtn) {
        cerr << "Error: Failed to load eye classifier: " << eye_classifier.first << endl;
    }

    return face_rtn && eye_rtn ? ReturnCodes::Success : ReturnCodes::Error;

}

ReturnCodes CamHandler::DetectAndDraw(cv::Mat& img) {
    // define some needed in between step vars
    std::vector<cv::Rect> faces;
    cv::Mat gray_img;

    // convert img to grayscale
    cv::cvtColor(img, gray_img, cv::COLOR_BGR2GRAY);
    // cv::equalizeHist(gray_img, gray_img);

    // detect faces of different sizes using cascade classifier
    facial_classifier.second.detectMultiScale(gray_img, faces, 1.3, 5);

    // draw circles around the faces
    for (auto& face : faces) {
        // center a point/circle in the middle of the object
        const cv::Point center(
            face.x + face.width/2,
            face.y + face.height/2
        );

        // actually draw circle (centered around face) on img
        cv::circle(
            img,                                // img to draw on
            center,                             // circle's center pt
            (face.width + face.height) / 3,     // radius
            cv::Scalar( 255, 0, 255 ),          // color (purple)
            4                                   // thickness
        );

        // detect & draw eyes in each face

        /* (Dont care about circling the eyes currently)
        cv::Mat faceROI = gray_img(face);
        std::vector<cv::Rect> eyes; // stores detected eyes
        eye_classifier.detectMultiScale(faceROI, eyes);

        for (auto& eye : eyes) {
            const cv::Point eye_center(
                face.x + eye.x + eye.width/2,
                face.y + eye.y + eye.height/2
            );
            
            // draw the actual circle around the eye
            const int radius = cvRound( (eye.width + eye.height) * 0.25 );
            cv::circle(
                img,
                eye_center,
                radius,
                cv::Scalar( 255, 0, 0 ),
                4
            );

        } // end of iteration over eyes in faces
        */

    } // end of iteration over faces

    return ReturnCodes::Success;
}


}; // end of Camera namespace

}; // end of RPI namespace