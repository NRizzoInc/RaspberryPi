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
 * @brief Final endpoint for sending packets to the web app backend which sends to server
 * @note see pkt_sample.json in network dir for what it should look like
 */
export const sendPkt = async (leds, motors, servos, camera) => {
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

/**
 * @brief Sends an LED pkt
 * @param {{
 *      "red":       Boolean,
 *      "yellow":    Boolean,
 *      "green":     Boolean,
 *      "blue":      Boolean,
 * }} leds
 */
export const sendLedPkt = async (leds) => {
    await sendPkt(leds, {}, {}, {})
}

/**
 * @brief Sends an motor pkt
 * @param {{
 *      "forward":  false,
 *      "backward": false,
 *      "right":    false,
 *      "left":     false
 * }} motors default to motors being off
 */
export const sendMotorPkt = async (motors) => {
    await sendPkt({}, motors, {}, {})
}

/**
 * @brief Sends an servo packet
 * @param {{
 *      "horiz":    0 | 1 | -1,
 *      "vert":     0 | 1 | -1
 * }} servos 
 * @argument servos horiz: right/left/unchanged
 * @argument servos vert: up/down/unchanged
 */
export const sendServoPkt = async (servos) => {
    await sendPkt({}, {}, servos, {})
}

/**
 * @brief Send a camera packet
 * @param {{
 *  "is_on": Boolean
 * }} camera
 * @argument is_on true: turn the camera on, false: turn the camera off
 */
export const sendCamPkt = async (camera) => {
    await sendPkt({}, {}, {}, camera)
}
