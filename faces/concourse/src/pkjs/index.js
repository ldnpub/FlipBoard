// FlipBoard CONCOURSE — phone side. Clay renders the settings page and, on save,
// auto-sends each value to the watch keyed by its messageKey. The custom theme
// function recolours Clay's accent to this face's colour (departure-board orange-red).
var Clay = require('pebble-clay');
var clayConfig = require('./config');

// Injected into the config page (Clay stringifies this), so it must be
// self-contained: no require/closures. Recolours the accent on AFTER_BUILD.
function flipboardTheme(minified) {
  var ACCENT = '#d65a3a';
  this.on(this.EVENTS.AFTER_BUILD, function () {
    var css =
      'strong{color:' + ACCENT + ' !important}' +
      '.button.primary,.button[type=submit],button.primary,button[type=submit]' +
      '{background-color:' + ACCENT + ' !important;-webkit-tap-highlight-color:' + ACCENT + '}' +
      'input[type=range]{accent-color:' + ACCENT + '}';
    var s = document.createElement('style');
    s.type = 'text/css';
    s.appendChild(document.createTextNode(css));
    document.head.appendChild(s);
  });
}

new Clay(clayConfig, flipboardTheme);
