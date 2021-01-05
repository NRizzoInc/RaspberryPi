'use strict';
/**
 * @file Handles the sending of packets to the web app to send to the rpi
 */

import { postPktData } from "./request_handler.js"


/**
 * @param {Number} val The value that you want to make sure is true/false
 * @returns {Boolean} Val covnerted to true/false (event if it already was true/false)
 */
const toBool = (val) => {
    return !!val
}

/**
 * 
 * @param {{
 * "red":       Boolean,
 * "yellow":    Boolean,
 * "green":     Boolean,
 * "blue":      Boolean,
 * } | {}} leds A json containing one of the color:bool pairs listed (defaults to empty json)
 */
export const sendPkt = async (
    leds={},
    motors={
        // default to motors being off
        "forward":  false,
        "backward": false,
        "right":    false,
        "left":     false
    },
    camera={
        "is_on":    true // have camera turn on when web app first connects
    }
) => {
    // handle sending data back to web app server
    // see pkt_sample.json in network dir for what it should look like
    const pkt = {
        "control": {
            "led"       : leds,
            "motor"     : motors,
            "camera"    : camera
        },
    }
    await postPktData(pkt)
} 