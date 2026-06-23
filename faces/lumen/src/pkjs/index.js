// FlipBoard LUMEN — phone side. Clay renders the settings page and, on save,
// auto-sends each value to the watch keyed by its messageKey (LANG, STEP_GOAL,
// GRID). The C side (inbox_received) applies them.
var Clay = require('pebble-clay');
var clayConfig = require('./config');
new Clay(clayConfig);
