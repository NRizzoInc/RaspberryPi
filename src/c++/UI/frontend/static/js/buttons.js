'use strict';
/**
 * @file Handles the toggling of led button colors
 */

import { sendPkt } from "./pkt.js"

const ColorList = ["red", "yellow", "green", "blue"]

/**
 * @brief helper function to convert a button's id to its actual color
 * @note Button's id's are of format: <color>-led-btn (cut off last chars on id ("-led-btn")
 * @param {String} btnId The id of the button element
 * @returns {"red" | "yellow" | "green" | "blue"}
 */
const btnIdToColor = (btnId) => btnId.slice(0, -8)


/**
 * @brief helper function to convert a button's color to its actual id
 * @note Button's id's are of format: <color>-btn
 * @param {"red" | "yellow" | "green" | "blue"} btn_color The color of the button
 * @returns {String} The id of the button element
 */
const btnColorToId = (btn_color) => `${btn_color}-led-btn`


// get all the buttons to keep track of their states
const led_btns = Array.from(document.getElementsByTagName("button"))
                    .filter((el) => el.id.endsWith("-led-btn")) // only add led buttons to list

const leds_state = led_btns.reduce((led_dict, el) => {
    // map each led to start off as off
    led_dict[btnIdToColor(el.id)] = false
    return led_dict
})

/**
 * 
 * @param {"red" | "yellow" | "green" | "blue"} btn_color The color of the button pressed
 */
const handleEvent = (btn_color) => {
    // toggle state
    leds_state[btn_color] = !leds_state[btn_color]
    sendPkt(leds_state, {}, {}, {})

    // if on, keep button darker/highlighted
    const darken_class = "btn-darken"
    const btn_el = document.getElementById(btnColorToId(btn_color))
    if (leds_state[btn_color]) {
        btn_el.classList.add(darken_class)
    } else {
        btn_el.classList.remove(darken_class)
    }
}

/**
 * @brief keyboard listener handler
 * @param {EventListenerObject} e The event obejct
 * @note keyCode list: https://keycode.info/
 */
const handleBtnsKeyboard = (e) => {
    switch (e.keyCode) {
        case 49: // '1' key
            handleEvent("red");
            break;
        case 50: // '2' key
            handleEvent("yellow");
            break;
        case 51: // '3' key
            handleEvent("green");
            break;
        case 52: // '4' key
            handleEvent("blue");
            break;
    }
}

// create event listener for each button that toggles the led's current state

// keyboard key listeners
document.addEventListener("keydown", (e) => handleBtnsKeyboard(e))

// mouse click listeners
led_btns.forEach( (el) => {
    el.addEventListener("click", () => {
        const color = btnIdToColor(el.id)
        handleEvent(color)
    })
})

