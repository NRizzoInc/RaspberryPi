'use strict';
/**
 * @file Very basic & simple js code to handle keyboard/mouse listener for d-pad
 * The arrow/wasd keys are responsible for the robot's motors & movement
 */

import { sendMotorPkt, sendServoPkt } from "./pkt.js"

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

// contains current servo status (0 = unchanged, or +/-#)
const servo_status = {
    "horiz":    0,
    "vert":     0
}

// controls how much each servo keypress effects the actual servo's position
let servo_sensitivity = 1

/**
 * @brief handles what happens after the pressing of a button via keyboard or mouse
 * @param {"forward" | "backward" | "left" | "right"} direction The direction being pressed
 * @param {Boolean} isDown True if key is currently being pressed, False if is not
 * @note keyCode list: https://keycode.info/
 */
const pressMotors = async (direction, isDown) => {
    // if down, motor should turn on (needs to be true/false for json to be parsable)
    //  -- lucky that js & c++ use same nomenclature)

    // only update and send new status if it is different from current status
    // (prevent sending duplicate packets)
    if (motors_status[direction] != isDown) {
        motors_status[direction] = isDown
        await sendMotorPkt(motors_status)
    }
}

/**
 * @param {"horiz" | "vert"} orient Which servo 
 * @param {0 | 1 | -1} val The value to assign the servo
 */
const pressServos = async (orient, val) => {
    servo_status[orient] = val*servo_sensitivity
    await sendServoPkt(servo_status)
}

/**
 * @brief keyboard listener handler
 * @param {EventListenerObject} e The event object
 * @param {Boolean} isDown True if key is currently being pressed, False if is not
 * @note keyCode list: https://keycode.info/
 */
const handleKeyboard = (e, isDown) => {
    switch (e.key) {
        // servo keypresses
        case "ArrowLeft":
            pressServos("horiz", isDown ? -1 : 0)
            break;
        case "ArrowRight":
            pressServos("horiz", isDown ?  1 : 0)
            break;
        case "ArrowUp":
            pressServos("vert", isDown ?  1 : 0)
            break;
        case "ArrowDown":
            pressServos("vert", isDown ? -1 : 0)
            break;

        // motor keypresses
        case "a":
            pressMotors("left", isDown);
            break;
        case "d":
            pressMotors("right", isDown);
            break;
        case "w":
            pressMotors("forward", isDown);
            break;
        case "s":
            pressMotors("backward", isDown);
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
