'use strict';
/**
 * @file Handles the continous updating of the video stream
 */

import { getJsonData, doesPageExist } from "./request_handler.js"
import { sendPkt } from "./pkt.js"


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
    const play_pause_els = [cam_vid, play_pause_btn]

    // represents what the camera control packet looks like
    const camera_status = {
        "is_on": true // have camera turn on when web app connects
    }

    // start off by initializing camera properly
    sendPkt({}, {}, camera_status)

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
            play_pause_btn.classList.add("pause-icon")
            play_pause_btn.classList.remove("play-icon")
        } else {
            // set to pause, show the play button
            play_pause_btn.classList.add("play-icon")
            play_pause_btn.classList.remove("pause-icon")
        }

        // update backend with camera status
        sendPkt({}, {}, camera_status)
    }
    // trigger toggle for starting condition to show pause button while removing blue box
    playpause_click(true)

    /**
     * @brief on click callback that JUST toggles play button
     * @param {Boolean} hide_play true if want to hide "play" icon and show pause icon (i.e. video is going to play)
     */
    const toggle_playpause_btn = () => playpause_click(null)

    // keep reloading JUST the image at the correct fps to mimic a video
    // if connection dies, stop trying to reload
    stop_dict.intervals.push(setInterval(
        async () => {
            // only refresh image if backend is still up
            // -- prevents issue with reloading to blank image and erroring
            const src_exist = await doesPageExist(cam_original_src, "GET")
            if (!src_exist) {
                // stop click event listeners
                stop_dict.listeners.forEach(type_el => type_el[1].removeEventListener(type_el[0], toggle_playpause_btn))

                // set play button to is paused
                playpause_click(false)

                // https://developer.mozilla.org/en-US/docs/Web/API/EventTarget/removeEventListener
                stop_dict.intervals.forEach(int_pid => clearInterval(int_pid))
                return // prevent latest frame from reloading
            }

            // attach random string to force reload of JUST the image
            const cache_refresh = `?v=${new Date().getTime()}`
            cam_vid.src = cam_original_src + cache_refresh
        }, 1000 / fps // need ms per frame
    ))

    // play/pause on clicking image or play/pause btn
    play_pause_els.forEach( (btn_el) => {
        // actually create onclick event listener & store it for eventual removal
        btn_el.addEventListener("click", toggle_playpause_btn)
        stop_dict.listeners.push(["click", btn_el])
    })
})
