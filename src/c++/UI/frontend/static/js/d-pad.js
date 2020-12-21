// Prevent scrolling on every click!
// credit: https://codepen.io/tswone/pen/GLzZLd
// super sweet vanilla JS delegated event handling!
document.body.addEventListener("click", function(e) {
	if(e.target && e.target.nodeName == "A") {
    e.preventDefault();
	}
});
let dpads = Array.prototype.slice.call(document.getElementsByClassName('d-pad'), 0),
      opads = Array.prototype.slice.call(document.getElementsByClassName('o-pad'), 0),
      els = dpads.concat(opads);
function dir(dir) {  
  for (let i = 0; i < els.length; i++) {
    const el = els[i],
          d = el.className.indexOf('d-') !== -1,
          what = d ? 'd-pad' : 'o-pad';
    console.log(what);
    el.className = what + ' ' + dir;
  }
}
document.body.onkeyup = function(e) {
  switch(e.which) {
    case 37: dir('left'); break;
    case 39: dir('right'); break;
    case 38: dir('up'); break;
    case 40: dir('down'); break;
  }
};