'use strict';
/**
 * @file Handles the continous updating of the video stream
 */

import { getJsonData } from "./request_handler.js"


// todo: add image title, hover, etc

$('document').ready( async () => {
    /**
     * Image representing the camera's most recent frame.
     * If refreshed often, forms video
     */
    const cam = document.getElementById("Cam-Stream")
    const cam_original_src = cam.src

    // get the camera's settings
    const cam_settings = await getJsonData("/Camera/settings.json")
    const fps = cam_settings.fps // TODO: get this from get request


    // keep reloading JUST the image at the correct fps to mimic a video
    window.setInterval(
        () => {
            // attach random string to force reload of JUST the image
            const cache_refresh = `?v=${new Date().getTime()}`
            cam.src = cam_original_src + cache_refresh
        }, 1000 / fps // need ms per frame
    )

})
