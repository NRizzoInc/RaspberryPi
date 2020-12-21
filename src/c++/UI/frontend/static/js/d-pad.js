// Prevent scrolling on every click!
// credit: https://codepen.io/tswone/pen/GLzZLd
// super sweet vanilla JS delegated event handling!
document.body.addEventListener("click", e => {
    const targetElement = e.target
    const targetName = targetElement.name
    if (targetElement && targetElement.nodeName == "A") {
        e.preventDefault();
    }
});

let dpads = Array.prototype.slice.call(document.getElementsByClassName('d-pad'), 0)
let opads = Array.prototype.slice.call(document.getElementsByClassName('o-pad'), 0)
let els = dpads.concat(opads);

// handles the pressing of a button
const press = (press_direction) => {
    for (let i = 0; i < els.length; i++) {
        let el = els[i]
        const ctrl_idx = el.className.indexOf('d-') !== -1
        const which_ctrler = ctrl_idx ? 'd-pad' : 'o-pad'
        // TODO: remove after debugging
        console.log(`${which_ctrler}: ${press_direction}`);
    }
}

// keyCode list: https://keycode.info/
document.body.onkeyup = (e) => {
    switch (e.keyCode) {
        case 65: // 'a' key
        case 37: // left arrow key
            press('left');
            break;
        case 68: // 'd' key
        case 39: // right arrow key
            press('right');
            break;
        case 87: // 'w' key
        case 38: // up arrow key
            press('up');
            break;
        case 83: // 's' key
        case 40: // down arrow key
            press('down');
            break;
    }
};