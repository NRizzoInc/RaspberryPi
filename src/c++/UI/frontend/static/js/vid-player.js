'use strict';
/**
 * @file Handles the continous updating of the video stream
 */

// todo: add image title, hover, etc

/**
 * Image representing the camera's most recent frame.
 * If refreshed often, forms video
 */
const cam = document.getElementById("Cam-Stream")
const cam_original_src = cam.src
const fps = 10 // TODO: get this from get request
window.setInterval(
    () => {
        // attach random string to force reload of JUST the image
        const cache_refresh = `?v=${new Date().getTime()}`
        cam.src = cam_original_src + cache_refresh
    }, 1000 / fps // need ms per frame
)
