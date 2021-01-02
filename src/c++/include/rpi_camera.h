#ifndef RPI_CAMERA_H
#define RPI_CAMERA_H

// Standard Includes
#include <iostream>
#include <string>
#include <thread> // for this_thread
#include <chrono> // for time units
#include <atomic>
#include <functional>

// Our Includes
#include "constants.h"
#include "timing.hpp"

// 3rd Party Includes
#include <raspicam_cv.h>
#include <opencv2/imgproc.hpp> // for putText()

namespace RPI {

namespace Camera {


// for convenience within this namesapce bc super long
using time_point = std::chrono::_V2::system_clock::time_point;

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
         * @param max_frame_count (defualts to -1 = infinite) If set, only this number of frames will be taken
         * @param should_init (default=true) Initialize obj in constructor 
         * (If false, you will have to call SetupCam() manually).
         * Needed if running client code on non-rpi w/o camera to open 
         */
        CamHandler(const int max_frame_count=-1, const bool should_init=true);
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
         * @brief Sets the Grab Callback function to use when a camera frame is grabbed
         * @param grab_cb The callback to use
         * @param grab_cb Returns: callback should be void return
         * @param grab_cb param: char vector containing the frames pixels (aka const std::vector<char>& frame)
         * @return ReturnCodes Success if set correctly
         */
        ReturnCodes setGrabCallback(std::function<void(const std::vector<char>& frame)> grab_cb);

        /********************************************* Camera Functions ********************************************/

        /**
         * @brief Opens the camera object and waits for it to stabilize
         * @returns Success if no issues
         */
        ReturnCodes OpenCam();

        /**
         * @brief Main function to start grabbing frames from the camera
         * @param should_save (default=true) If true, saves the last grabbed frame to disk
         */
        void RunFrameGrabber(const bool should_save=true);

    private:
        /******************************************** Private Variables ********************************************/

        int                                                 frame_count;   // current number of grabbed frames
        const int                                           max_frames;    // the max # frames to grab (-1 = infinite)
        std::atomic_bool                                    stop_grabbing; // if true, the grabbing loop will stop
        time_point                                          start_time;    // when camera started grabbing
        std::function<void(const std::vector<char>& frame)> grab_cb;       // callback to use when a frame is grabbed

        /********************************************* Helper Functions ********************************************/

        ReturnCodes SetupCam();

}; // end of CamHandler class

}; // end of Camera namespace

}; // end of RPI namespace


#endif
