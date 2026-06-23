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
          'Labels follow the language you pick here. You can also shake your ' +
          'wrist to cycle to the next language.'
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
  { type: 'submit', defaultValue: 'Save settings' }
];
