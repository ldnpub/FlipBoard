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
          'Labels (STEPS, DATE, BATTERY) follow the language you pick here. You ' +
          'can also shake your wrist to cycle to the next language.'
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
  { type: 'submit', defaultValue: 'Save settings' }
];
