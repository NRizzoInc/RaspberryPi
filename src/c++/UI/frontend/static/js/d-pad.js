// Prevent scrolling on every click!
// credit: https://codepen.io/tswone/pen/GLzZLd
// super sweet vanilla JS delegated event handling!
document.body.addEventListener("click", e => {
    const targetElement = e.target
    const targetName = targetElement.name
    if (targetElement && targetElement.nodeName == "A") {
        e.preventDefault();
    }
    // TODO: remove after done debugging
    console.log(`Triggered: ${targetName}`)
});

let dpads = Array.prototype.slice.call(document.getElementsByClassName('d-pad'), 0)
let opads = Array.prototype.slice.call(document.getElementsByClassName('o-pad'), 0)
let els = dpads.concat(opads);

// handles the pressing of a button
const press = (press_direction) => {
    for (let i = 0; i < els.length; i++) {
        const el = els[i],
            d = el.className.indexOf('d-') !== -1,
            what = d ? 'd-pad' : 'o-pad';
        console.log(what);
        el.className = what + ' ' + press_direction;
    }
}

document.body.onkeyup = (e) => {
    switch (e.which) {
        case 37:
            press('left');
            break;
        case 39:
            press('right');
            break;
        case 38:
            press('up');
            break;
        case 40:
            press('down');
            break;
    }
};