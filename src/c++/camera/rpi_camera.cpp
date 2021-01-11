#include "rpi_camera.h"

namespace RPI {

namespace Camera {

// for convenience
using std::cout;
using std::cerr;
using std::endl;

/********************************************** Constructors **********************************************/

CamHandler::CamHandler(const int max_frame_count, const bool should_init)
    : raspicam::RaspiCam_Cv{}
    , frame_count{0}
    , max_frames{max_frame_count}       // defaults to infinite = -1
    , stop_thread{false}
    , should_record{false}
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
    bool rtn {true};

    // TODO: make sure is using h264 raw encoding to mp4
    rtn &= RaspiCam_Cv::set( CV_CAP_PROP_FORMAT, CV_8UC1 );
    rtn &= RaspiCam_Cv::set( CV_CAP_PROP_FORMAT, CV_8UC3 );
    rtn &= RaspiCam_Cv::set( CV_CAP_PROP_FRAME_WIDTH, Constants::Camera::FRAME_WIDTH );
    rtn &= RaspiCam_Cv::set( CV_CAP_PROP_FRAME_HEIGHT, Constants::Camera::FRAME_HEIGHT );
    rtn &= RaspiCam_Cv::set( CV_CAP_PROP_FPS, Constants::Camera::VID_FRAMERATE );

    // open camera after setting correct settings
    if(OpenCam() != ReturnCodes::Success) {
        cerr << "Failed to open camera" << endl;
        return ReturnCodes::Error;
    }

    // load facial recognition classifiers
    rtn &= LoadClassifiers() == ReturnCodes::Success;

    return rtn ? ReturnCodes::Success : ReturnCodes::Error;
}


ReturnCodes CamHandler::LoadClassifiers() {
    // TODO: use cli to path these files
    // these paths/files comes from the opencv package
    fs::path classifiers_dir            {"/usr/share/opencv/haarcascades/"};
    fs::path facial_classifier_file     {classifiers_dir / "haarcascade_frontalcatface.xml"};
    fs::path obstruct_classifier_file   {classifiers_dir / "haarcascade_eye_tree_eyeglasses.xml"};
    const bool face_rtn                 {facial_classifier.load(facial_classifier_file.string())};
    const bool obstruct_rtn             {obstruct_classifier.load(obstruct_classifier_file.string())};

    // check rtn codes
    if(!face_rtn) {
        cerr << "Error: Failed to load facial recognition classifier: " << facial_classifier_file << endl;
    }

    if (!obstruct_rtn) {
        cerr << "Error: Failed to load obstruction recognition classifier: " << obstruct_classifier_file << endl;
    }

    return face_rtn && obstruct_rtn ? ReturnCodes::Success : ReturnCodes::Error;

}


}; // end of Camera namespace

}; // end of RPI namespace