// FlipBoard — phone side. Clay renders the settings page and, on save,
// auto-sends each item's value to the watch keyed by its messageKey
// (LANG, STEP_GOAL). The C side (flipboard.h s_flip_inbox) applies them.
var Clay = require('pebble-clay');
var clayConfig = require('./config');
new Clay(clayConfig);
