'use strict';
/**
 * @file Handles the continous updating of the video stream
 */

import { getJsonData, doesPageExist } from "./request_handler.js"
import { sendCamPkt } from "./pkt.js"


// todo: add image title, hover, etc

$("document").ready( async () => {
    /**
     * Image representing the camera's most recent frame.
     * If refreshed often, forms video
     */
    const cam_vid = document.getElementById("Cam-Stream")
    const cam_original_src = cam_vid.src

    // get the camera's settings
    const cam_settings = await getJsonData("/Camera/settings.json")
    const fps = cam_settings.fps // TODO: get this from get request

    // track/manage camera's recording status
    // contains all elements which when clicked toggle recording
    const play_pause_btn = document.getElementById("play-pause-cam-btn")
    const play_pause_icon = document.getElementById("play-pause-cam-icon")
    const play_pause_els = [cam_vid, play_pause_btn]

    // represents what the camera control packet looks like
    const camera_status = {
        "is_on": true // have camera turn on when web app connects
    }

    // start off by initializing camera properly
    sendCamPkt(camera_status)

    /**************************************** Event Listeners ****************************************************/

    // dict containing lists of listeners and intervals to stop if backend goes down
    const stop_dict = {
        "intervals": [],
        "listeners": [] // [[type, el], ...]
    }

    /**
     * @brief on click callback that either toggles or specifically sets which button to show
     * (needed ahead of time to remove it as well)
     * @param {Boolean} hide_play true if want to hide "play" icon and show pause icon (i.e. video is going to play)
     * @note Call `toggle_playpause_btn` over this function
     */
    const playpause_click = (hide_play=null) => {
        // toggle play/pause buttons
        camera_status.is_on = hide_play != null ? hide_play : !camera_status.is_on 

        if (camera_status.is_on) {
            // set to play, show the pause button
            play_pause_icon.classList.add("fa-pause")
            play_pause_icon.classList.remove("fa-play")
        } else {
            // set to pause, show the play button
            play_pause_icon.classList.add("fa-play")
            play_pause_icon.classList.remove("fa-pause")
        }

        // update backend with camera status
        sendCamPkt(camera_status)
    }

    /**
     * @brief on click callback that JUST toggles play button
     * @param {Boolean} hide_play true if want to hide "play" icon and show pause icon (i.e. video is going to play)
     */
    const toggle_playpause_btn = () => playpause_click(null)

    
    let isCamStopped = false
    let WakeupInterval = null // stores reference to interval that checks if page is up
    // loops and checks if page has come back up yet, if so, it start everything up
    const CreateWakeupInterval = () => {
        // only create interval if dne
        if (WakeupInterval != null) return

        WakeupInterval = setInterval( 
            async () => {
                // if camera is currently going, there is nothing to do
                if (!isCamStopped) return

                // if camera currently stopped, but the page is back up, then restart
                else if (await doesPageExist(cam_original_src, "GET")) {
                    StartCamActivities()
                    clearInterval(WakeupInterval)
                    WakeupInterval = null
                }
            }, 3000 // check every 3 seconds
        )
    }

    // responsible for stopping all listeners and intervals working on behalf of the camera/video
    const StopCamActivites = () => {
        // dont stop if already stopped
        if (isCamStopped) return

        // stop click event listeners
        stop_dict.listeners.forEach(type_el => type_el[1].removeEventListener(type_el[0], toggle_playpause_btn))

        // set play button to is paused
        playpause_click(false)

        // https://developer.mozilla.org/en-US/docs/Web/API/EventTarget/removeEventListener
        stop_dict.intervals.forEach(int_pid => clearInterval(int_pid))

        // reset stop lists
        // fast and efficient way to clear array: https://stackoverflow.com/a/1232046/13933174
        stop_dict.listeners.length = 0
        stop_dict.intervals.length = 0
        isCamStopped = true

        // start up wakeup routine to wait for moment to restart
        CreateWakeupInterval()
    }

    // creates the interval which continously grabs frames to generate the "video"
    // wrap in function so interval can be recreated when/if stopped
    // keep reloading JUST the image at the correct fps to mimic a video
    const CreateVidStream = () => {
        // trigger toggle for starting condition to show pause button while removing blue box
        playpause_click(true)

        stop_dict.intervals.push(setInterval(
            async () => {
                // attach random string to force reload of JUST the image
                const cache_refresh = `?v=${new Date().getTime()}`
                const new_img_url = cam_original_src + cache_refresh

                // if connection dies, dont try to reload image (keep last)
                // only refresh image if backend is still up
                // -- prevents issue with reloading to blank image and erroring
                const src_exist = await doesPageExist(new_img_url, "GET")
                if (!src_exist) {
                    StopCamActivites()
                    return // prevent latest frame from reloading
                }

                // actually reload page
                cam_vid.src = new_img_url

            }, 1000 / fps // need ms per frame
        ))
    }

    // responsible for creating btn listeners in regards to the camera/video
    const CreateCamBtnListners = () => {
        // play/pause on clicking image or play/pause btn
        play_pause_els.forEach( (btn_el) => {
            // actually create onclick event listener & store it for eventual removal
            btn_el.addEventListener("click", toggle_playpause_btn)
            stop_dict.listeners.push(["click", btn_el])
        })
    }

    // responsible for starting all listeners and intervals working on behalf of the camera/video
    const StartCamActivities = () => {
        CreateVidStream()
        CreateCamBtnListners()
        isCamStopped = false
    }

    // start up page to begin with
    StartCamActivities()
})
