#ifndef RPI_CAMERA_H
#define RPI_CAMERA_H

// Standard Includes
#include <iostream>
#include <string>
#include <thread> // for this_thread
#include <chrono> // for time units
#include <atomic>
#include <functional>
#include <experimental/filesystem> // to get path to classifier files

// Our Includes
#include "constants.h"
#include "timing.hpp"

// 3rd Party Includes
#include <raspicam_cv.h>
#include <opencv2/imgproc.hpp> // for putText()
#include <opencv2/objdetect.hpp> // for object detection

namespace RPI {

namespace Camera {

// filesystem namespace shortened for convenience bc super long & experimental
namespace fs = std::experimental::filesystem;

// for convenience within this namesapce bc super long
using time_point = std::chrono::_V2::system_clock::time_point;

using GrabFrameCb = std::function<void(const std::vector<unsigned char>& frame)>;

using Classifier = std::pair<const fs::path, cv::CascadeClassifier>;

/**
 * @brief Extends the raspicam opencv camera class
 * @note use `isOpened()` to check open status
 * @note Based on https://github.com/cedricve/raspicam Raspicam_Cv examples
 */
class CamHandler : public raspicam::RaspiCam_Cv {

    public:
        /********************************************** Constructors **********************************************/

        /**
         * @brief Construct a new camera handler object
         * @param verbosity If true, will print more information that is strictly necessary
         * @param max_frame_count (defualts to -1 = infinite) If set, only this number of frames will be taken
         * @param face_xml (default to local version) Absolute path to the opencv face classifier xml file
         * @param eye_xml  (default to local version) Absolute path to the opencv eye classifier xml file
         * @param should_init (default=true) Initialize obj in constructor 
         * (If false, you will have to call SetupCam() manually).
         * Needed if running client code on non-rpi w/o camera to open 
         */
        CamHandler(
            const bool verbosity=false,
            const int max_frame_count=-1,
            const bool should_init=true,
            const std::string face_xml="",
            const std::string eye_xml=""
        );
        virtual ~CamHandler();

        /********************************************* Getters/Setters *********************************************/

        /**
         * @brief Checks if grabbing loop should end/stop
         * @return true Should stop
         * @return false Should keep going
         */
        bool getShouldStop() const;

        /**
         * @brief Set whether or not the grabbing loop should end/stop
         * @return ReturnCodes Success if no issues
         */
        ReturnCodes setShouldStop(const bool new_status);

        /**
         * @brief Checks if camera should be actively capturing frames
         * @return true if camera should be recording
         * @return false if camera should stop/not be recording
         */
        bool getShouldRecord() const;

        /**
         * @brief Set whether or not the camera should be actively capturing frames
         * @return ReturnCodes Success if no issues
         */
        ReturnCodes setShouldRecord(const bool new_status);

        /**
         * @brief Sets the Grab Callback function to use when a camera frame is grabbed
         * @param grab_cb The callback to use
         * @param grab_cb Returns: callback should be void return
         * @param grab_cb param: char vector containing the frames pixels (aka const std::vector<unsigned char>& frame)
         * @return ReturnCodes Success if set correctly
         */
        ReturnCodes setGrabCallback(GrabFrameCb grab_cb);

        /********************************************* Camera Functions ********************************************/

        /**
         * @brief Opens the camera object and waits for it to stabilize
         * @returns Success if no issues
         */
        ReturnCodes OpenCam();

        /**
         * @brief Main function to start grabbing frames from the camera
         * @param record_immed True if camera should start capturing frames immediately
         * @param should_save (default=true) If true, saves the last grabbed frame to disk
         */
        void RunFrameGrabber(const bool record_immed, const bool should_save=true);

    private:
        /******************************************** Private Variables ********************************************/

        const bool                  is_verbose;    // false if should only print errors/important info
        int                         frame_count;   // current number of grabbed frames
        const int                   max_frames;    // the max # frames to grab (-1 = infinite)
        std::atomic_bool            stop_thread;   // if true, the grabbing thread will stop
        std::atomic_bool            should_record; // if true, thread keeps going but grabbing will stop
        time_point                  start_time;    // when camera started grabbing
        GrabFrameCb                 grab_cb;       // callback to use when a frame is grabbed

        // PreDefined/Trained Object Detection Classifiers (Facial Recognition)
        Classifier                  facial_classifier;
        Classifier                  eye_classifier;          // classifier for objects that object face

        /********************************************* Helper Functions ********************************************/

        /**
         * @brief Sets up the camera properties & settings prior to recording
         * @return ReturnCodes Success if no issues
         */
        ReturnCodes SetupCam();

        /*************************************** Facial Recognition Functions **************************************/

        /**
         * @brief Loads the classifier files for facial detection
         * @return ReturnCodes Sucess if no issues
         */
        ReturnCodes LoadClassifiers();

        // uses pairing to easily load the file into the cv obj
        ReturnCodes LoadClassifiers(Classifier& classifiers_pair);

        /**
         * @brief Performs facial recognition on the passed image using preloaded classifiers
         * @param img The image to detect faces on & modify (hence not const)
         * @return ReturnCodes Success if no issues
         */
        ReturnCodes DetectAndDraw(cv::Mat& img);

}; // end of CamHandler class

}; // end of Camera namespace

}; // end of RPI namespace


#endif
