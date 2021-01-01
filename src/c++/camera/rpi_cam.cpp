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
    , stop_grabbing{false}
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
    return stop_grabbing.load();
}

/**
 * @brief Set whether or not the grabbing loop should end/stop
 * @return ReturnCodes Success if no issues
 */
ReturnCodes CamHandler::setShouldStop(const bool new_status) {
    stop_grabbing.store(new_status);
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


void CamHandler::RunFrameGrabber() {
    cout << "Capturing "
         << (max_frames == -1 ? "infinite" : std::to_string(max_frames))
         << " frames" << endl;

    // place to store the grabbed frame
    cv::Mat image;

    // start capture
    // loop until max frame count or told to stop
    start_time      = std::chrono::system_clock::now();
    auto curr_time  = start_time;
    cout << "Starting Capture " << Helpers::Timing::GetCurrTimecode(start_time) << endl;
    while (!getShouldStop() && (max_frames == -1 || frame_count < max_frames)) {

        RaspiCam_Cv::grab();
        RaspiCam_Cv::retrieve(image);

        // add timestamp to frame
        curr_time               = std::chrono::system_clock::now();
        std::string timecode    { Helpers::Timing::GetCurrTimecode(curr_time) };
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

        // TODO: optional arg to print frame count? (--verbose?)
    }

    // stop
    cout << "Stopping Camera: " << Helpers::Timing::GetCurrTimecode(curr_time) << endl;
    RaspiCam_Cv::release();


    // save video (TODO: eventually turn to stream)
    constexpr auto filepath {"raspicam_cv_image.jpg"};
	cv::imwrite(filepath, image);
	cout << "Image saved at " << filepath << endl;
}

/********************************************* Helper Functions ********************************************/

ReturnCodes CamHandler::SetupCam() {
    // set camera properties
    // TODO: make sure is using h264 raw encoding to mp4
    RaspiCam_Cv::set( CV_CAP_PROP_FORMAT, CV_8UC1 );
    RaspiCam_Cv::set( CV_CAP_PROP_FORMAT, CV_8UC3 );
    RaspiCam_Cv::set( CV_CAP_PROP_FRAME_WIDTH, Constants::Camera::VID_WIDTH );
    RaspiCam_Cv::set( CV_CAP_PROP_FRAME_HEIGHT, Constants::Camera::VID_HEIGHT );
    RaspiCam_Cv::set( CV_CAP_PROP_FPS, Constants::Camera::VID_FRAMERATE );

    // open camera after setting correct settings
    if(OpenCam() != ReturnCodes::Success) {
        cerr << "Failed to open camera" << endl;
        return ReturnCodes::Error;
    }

    return ReturnCodes::Success;
}


}; // end of Camera namespace

}; // end of RPI namespace