// FlipBoard STIPPLE — phone settings page (Clay): language + daily step goal.
module.exports = [
  { type: 'heading', defaultValue: 'FlipBoard STIPPLE' },
  {
    type: 'text',
    defaultValue:
      'A FlipBoard departure-board watchface. Below the clock it shows your ' +
      'daily steps, the date, and the battery level.'
  },
  {
    type: 'section',
    items: [
      { type: 'heading', defaultValue: 'Steps' },
      {
        type: 'text',
        defaultValue:
          'The step count is colour-coded against your daily goal: warm red near ' +
          'zero, through amber and yellow around the middle, to green as you reach ' +
          'the goal. Set your goal below.'
      },
      { type: 'slider', messageKey: 'STEP_GOAL', defaultValue: 10000,
        label: 'Daily step goal', description: 'Steps per day that counts as 100 percent.',
        min: 1000, max: 30000, step: 500 }
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
      { type: 'select', messageKey: 'LANG', defaultValue: '0', label: 'Language',
        options: [
          { label: 'Francais', value: '0' }, { label: 'English', value: '1' },
          { label: 'Deutsch', value: '2' }, { label: 'Espanol', value: '3' },
          { label: 'Italiano', value: '4' }, { label: 'Nederlands', value: '5' },
          { label: 'Portugues', value: '6' }, { label: 'Polski', value: '7' },
          { label: 'Svenska', value: '8' }
        ] }
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
