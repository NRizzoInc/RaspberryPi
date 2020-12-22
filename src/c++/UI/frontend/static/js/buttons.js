'use strict';
/**
 * @file Handles the toggling of led button colors
 */

import { sendPkt } from "./pkt.js"

/**
 * @brief helper function to convert a button's id to its actual color
 * @note Button's id's are of format: <color>-btn
 * @param {String} btnId The id of the button element
 * @returns {"red" | "yellow" | "green" | "blue"}
 */
 const btnIdToColor = (btnId) => {
    //  cut off last 4 chars on id ("-btn")
    return btnId.slice(0, -4)
}

// get all the buttons to keep track of their states
const led_btns = Array.from(document.getElementsByTagName("button"))
const leds_state = led_btns.reduce((led_dict, el) => {
    // map each led to start off as off
    led_dict[btnIdToColor(el.id)] = false
    return led_dict
})

// create event listener for each button that toggles the led's current state
led_btns.forEach( (el) => {
    el.addEventListener("click", () => {
        // toggle state
        const color = btnIdToColor(el.id)
        leds_state[color] = !leds_state[color]
        sendPkt(leds_state)
    })
})

