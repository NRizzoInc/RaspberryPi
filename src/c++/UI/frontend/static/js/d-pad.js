'use strict';
/**
 * @file Very basic & simple js code to handle keyboard/mouse listener for d-pad
 */

import { postPktData } from "./request_handler.js"

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

// TODO: eventually convert to "motor"
const dir_to_led = {
    "left"  : "red",
    "right" : "yellow",
    "up"    : "green",
    "down"  : "blue",
}

/**
 * @brief handles what happens after the pressing of a button via keyboard or mouse
 * @param {"up" | "down" | "left" | "right"} direction The direction being pressed
 * @param {Boolean} isDown True if key is currently being pressed, False if is not
 * @note keyCode list: https://keycode.info/
 */
const press = async (direction, isDown) => {
    // parse data
    const led_color = dir_to_led[direction]

    // handle sending data back to web app server
    // see pkt_sample.json in network dir for what it should look like
    const pkt = {
        "control": {
            "led": {
                // dont set unknown colors bc dont know their current state
                // has to be false/true, not 0/1
                // "red":      false/true,
                // "yellow":   false/true,
                // "green":    false/true,
                // "blue":     false/true
            }
        },
    }
    // if down, led should turn on (needs to be true/false for json to be parsable)
    //  -- lucky that js & c++ use same nomenclature)
    pkt.control.led[led_color] = isDown
    await postPktData(pkt)
}

/**
 * @brief keyboard listener handler
 * @param {EventListenerObject} e The event obejct
 * @param {Boolean} isDown True if key is currently being pressed, False if is not
 * @note keyCode list: https://keycode.info/
 */
const handleKeyboard = (e, isDown) => {
    switch (e.keyCode) {
        case 65: // 'a' key
        case 37: // left arrow key
            press("left", isDown);
            break;
        case 68: // 'd' key
        case 39: // right arrow key
            press("right", isDown);
            break;
        case 87: // 'w' key
        case 38: // up arrow key
            press("up", isDown);
            break;
        case 83: // 's' key
        case 40: // down arrow key
            press("down", isDown);
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
document.body.onkeydown     = (e) => handleKeyboard(e, true)
document.body.onkeyup       = (e) => handleKeyboard(e, false)
document.body.onmousedown   = (e) => handleMouse(e, true)
document.body.onmouseup     = (e) => handleMouse(e, false)
