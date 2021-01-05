'use strict';
/**
 * @file Handles the continous updating of the video stream
 */

import { getJsonData } from "./request_handler.js"
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
    const play_btn = document.getElementById("Cam-PlayPause")
    // contains all elements which when clicked toggle recording
    const play_pause_els = [cam_vid, play_btn]
    // represents what the camera control packet looks like
    const camera_status = {
        "is_on": true // have camera turn on when web app connects
    } 

    /**************************************** Event Listeners ****************************************************/

    // keep reloading JUST the image at the correct fps to mimic a video
    window.setInterval(
        () => {
            // attach random string to force reload of JUST the image
            const cache_refresh = `?v=${new Date().getTime()}`
            cam_vid.src = cam_original_src + cache_refresh
        }, 1000 / fps // need ms per frame
    )

    // play/pause on clicking image or play/pause btn
    play_pause_els.forEach( (el) => {
        el.addEventListener("click", (el) => {
            camera_status.is_on = !camera_status.is_on
            sendPkt({}, {}, camera_status)
        })
    })
})
