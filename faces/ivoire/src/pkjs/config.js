// FlipBoard IVOIRE — phone settings page (Clay).
// Language selector, daily step-goal slider, and a description of the face —
// in particular how the step number is colour-coded.
module.exports = [
  { type: 'heading', defaultValue: 'FlipBoard IVOIRE' },
  {
    type: 'text',
    defaultValue:
      'A split-flap departure-board watchface. Two ivory flip tiles show the ' +
      'hour and minutes and fold over on each minute change, the way an airport ' +
      'board does. Below sit your daily steps, the date, and the battery level.'
  },
  {
    type: 'section',
    items: [
      { type: 'heading', defaultValue: 'Steps' },
      {
        type: 'text',
        defaultValue:
          'The step count at the top is colour-coded against your daily goal so ' +
          'you can read your progress at a glance: warm red when you are near ' +
          'zero, through amber and yellow around the middle, to green as you ' +
          'reach the goal. Set the goal that suits you below.'
      },
      {
        type: 'slider',
        messageKey: 'STEP_GOAL',
        defaultValue: 10000,
        label: 'Daily step goal',
        description: 'Steps per day that counts as 100%.',
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
          'Labels (STEPS, DATE, BATTERY) follow the language you pick here.'
      },
      {
        type: 'select',
        messageKey: 'LANG',
        defaultValue: '0',
        label: 'Language',
        options: [
          { label: 'Francais',    value: '0' },
          { label: 'English',     value: '1' },
          { label: 'Deutsch',     value: '2' },
          { label: 'Espanol',     value: '3' },
          { label: 'Italiano',    value: '4' },
          { label: 'Nederlands',  value: '5' },
          { label: 'Portugues',   value: '6' },
          { label: 'Polski',      value: '7' },
          { label: 'Svenska',     value: '8' }
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
