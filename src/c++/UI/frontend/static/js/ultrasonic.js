'use strict';
/**
 * @file Handles getting the ultrasonic data and displaying it on the web app gui page
 */

import { getUltrasonicData, getCamSettings } from "./pkt.js"

/*************************** manage the ultrasonic sensor's data ***************************/

// controls & stores the servo's sensitivity via arrow keys
const ultrasonic_el = document.getElementById("ultrasonic-dist")
const ultrasonic_beg_msg = ultrasonic_el.innerHTML // save prior to editing and adding actual value


// updates the div where the distance should be displayed with the current sensor value
const handleDistDiv = async () => {
    const cur_ultrasonic_data = await getUltrasonicData()
    // empty if error
    const cur_dist = cur_ultrasonic_data == {} ? -1 : cur_ultrasonic_data.dist
    ultrasonic_el.innerHTML = `${ultrasonic_beg_msg} ${cur_dist}`
}

/********* create/manage interval to every so often update the div with latest value *********/

// stores ultrasonic interval pid
let ultraInterval = null

const startUltraInternval = async () => {
    if (ultraInterval == null) {
        // update matching fps
        const fps = (await getCamSettings()).fps
        ultraInterval = setInterval(handleDistDiv, 1000 / fps)
    }
}

const stopUltraInterval = () => {
    if (ultraInterval != null) {
        clearInterval(ultraInterval)
    } 
}


$("document").ready( async () => {
    // call fn immediately to set value (should update auto)
    handleDistDiv()
    startUltraInternval()
})
