'use strict';
/**
 * @file Very basic & simple js code to handle keyboard/mouse listener for d-pad
 */

import { sendPkt } from "./pkt.js"

/**
 * @brief Gets which controller is being used based on element
 * @param {*} el The element
 * @returns {"d-pad" | "o-pad"} 
 */
const getCtrler = (el) => {
    const ctrl_idx = el.className.indexOf('d-') !== -1
    const which_ctrler = ctrl_idx ? 'd-pad' : 'o-pad'
    return which_ctrler
}

const ctrlers = ["d-pad", "o-pad"]
    .map((class_name) => Array.from(document.getElementsByClassName(class_name)))
    .reduce((btns_list, curr_btn_coll) => btns_list.concat(curr_btn_coll))

// contains current motor status (true = on, false = off)
// inits to off
const motors_status = {
    "forward":  false,
    "backward": false,
    "right":    false,
    "left":     false
}

/**
 * @brief handles what happens after the pressing of a button via keyboard or mouse
 * @param {"forward" | "backward" | "left" | "right"} direction The direction being pressed
 * @param {Boolean} isDown True if key is currently being pressed, False if is not
 * @note keyCode list: https://keycode.info/
 */
const press = async (direction, isDown) => {
    // if down, motor should turn on (needs to be true/false for json to be parsable)
    //  -- lucky that js & c++ use same nomenclature)
    motors_status[direction] = isDown
    console.log(`motors_status: ${JSON.stringify(motors_status)}`)
    await sendPkt({}, motors_status)
}

/**
 * @brief keyboard listener handler
 * @param {EventListenerObject} e The event object
 * @param {Boolean} isDown True if key is currently being pressed, False if is not
 * @note keyCode list: https://keycode.info/
 */
const handleKeyboard = (e, isDown) => {
    switch (e.key) {
        case "a":
        case "ArrowLeft":
            press("left", isDown);
            break;
        case "d":
        case "ArrowRight":
            press("right", isDown);
            break;
        case "w":
        case "ArrowUp":
            press("forward", isDown);
            break;
        case "s":
        case "ArrowDown":
            press("backward", isDown);
            break;
    }
}

// mouse listener
// Prevent scrolling on every click!
const handleMouse = (e, isDown) => {
    const targetElement = e.target
    const targetClass = targetElement.className // maps to direction for press()
    if (targetElement && targetElement.nodeName == "A") {
        e.preventDefault();
    }
    press(targetClass, isDown)
}

// actually create listeners
document.addEventListener("keydown",    (e) => handleKeyboard(e, true))
document.addEventListener("keyup",      (e) => handleKeyboard(e, false))
ctrlers.forEach( (btn_el) => {
    btn_el.addEventListener("mousedown",  (e) => handleMouse(e, true))
    btn_el.addEventListener("mouseup",    (e) => handleMouse(e, false))
})
