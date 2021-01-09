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
 * @note see pkt_sample.json in network dir for what it should look like
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
    servos={
        "horiz":    0, // +1/-1/0 (right/left/unchanged)
        "vert":     0  // +1/-1/0 (up/down/unchanged)
    },
    camera={}
) => {
    // handle sending data back to web app server
    const pkt = {
        "control": {
            "led"       : leds,
            "motor"     : motors,
            "servo"     : servos,
            "camera"    : camera
        },
    }
    await postPktData(pkt)
} 