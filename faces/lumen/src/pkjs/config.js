// FlipBoard LUMEN — phone settings page (Clay): language, daily step goal,
// and the LED ghost-grid intensity (0-100%).
module.exports = [
  { type: 'heading', defaultValue: 'FlipBoard LUMEN' },
  {
    type: 'text',
    defaultValue:
      'An LED dot-matrix departure board. Lit dots glow; the unlit dots form a ' +
      'faint ghost grid. Below the clock: your steps, the date, and the battery.'
  },
  {
    type: 'section',
    items: [
      { type: 'heading', defaultValue: 'Display' },
      {
        type: 'text',
        defaultValue:
          'Ghost-grid intensity sets how visible the unlit LEDs are. 0 percent ' +
          'hides the grid for maximum contrast (only the lit digits show); 100 ' +
          'percent shows the full grid. Tune it to taste on your watch.'
      },
      {
        type: 'slider',
        messageKey: 'GRID',
        defaultValue: 45,
        label: 'Ghost-grid intensity',
        description: '0 = no grid (max contrast), 100 = full grid.',
        min: 0,
        max: 100,
        step: 5
      }
    ]
  },
  {
    type: 'section',
    items: [
      { type: 'heading', defaultValue: 'Steps' },
      {
        type: 'text',
        defaultValue:
          'The step count is colour-coded against your goal: red near zero, ' +
          'through amber, to green as you reach it. Set your goal below.'
      },
      {
        type: 'slider',
        messageKey: 'STEP_GOAL',
        defaultValue: 10000,
        label: 'Daily step goal',
        description: 'Steps per day that counts as 100 percent.',
        min: 1000,
        max: 30000,
        step: 500
      }
    ]
  },
  {
    type: 'section',
    items: [
      { type: 'heading', defaultValue: 'Language' },
      {
        type: 'text',
        defaultValue:
          'Labels follow the language you pick here.'
      },
      {
        type: 'select',
        messageKey: 'LANG',
        defaultValue: '0',
        label: 'Language',
        options: [
          { label: 'Francais',   value: '0' },
          { label: 'English',    value: '1' },
          { label: 'Deutsch',    value: '2' },
          { label: 'Espanol',    value: '3' },
          { label: 'Italiano',   value: '4' },
          { label: 'Nederlands', value: '5' },
          { label: 'Portugues',  value: '6' },
          { label: 'Polski',     value: '7' },
          { label: 'Svenska',    value: '8' }
        ]
      }
    ]
  },
  {
    type: 'section',
    items: [
      { type: 'heading', defaultValue: 'Date format' },
      {
        type: 'select',
        messageKey: 'DATE_FMT',
        defaultValue: '0',
        label: 'Date format',
        options: [
          { label: 'Auto (with language)', value: '0' },
          { label: 'DD/MM (29/06)',        value: '1' },
          { label: 'MM/DD (06/29)',        value: '2' },
          { label: 'DD.MM (29.06)',        value: '3' },
          { label: 'YYYY-MM-DD (ISO)',     value: '4' }
        ]
      },
      {
        type: 'select',
        messageKey: 'YEAR_FMT',
        defaultValue: '0',
        label: 'Show year',
        options: [
          { label: 'Off',            value: '0' },
          { label: '2-digit (26)',   value: '1' },
          { label: '4-digit (2026)', value: '2' }
        ]
      }
    ]
  },
  {
    type: 'section',
    items: [
      { type: 'heading', defaultValue: 'Shake' },
      {
        type: 'text',
        defaultValue:
          'Choose what a wrist shake does. Off by default so it never triggers ' +
          'by accident while you walk.'
      },
      {
        type: 'select',
        messageKey: 'SHAKE_ACT',
        defaultValue: '0',
        label: 'Shake action',
        options: [
          { label: 'Off',                 value: '0' },
          { label: 'Replay flip',         value: '1' },
          { label: 'Backlight',           value: '2' },
          { label: 'Steps to goal',       value: '3' },
          { label: 'Random destination',  value: '4' },
          { label: 'Toggle 12h / 24h',    value: '5' }
        ]
      }
    ]
  },
  { type: 'submit', defaultValue: 'Save settings' }
];
