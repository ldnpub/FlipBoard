// FlipBoard SOLARI — phone settings page (Clay): language + daily step goal.
module.exports = [
  { type: 'heading', defaultValue: 'FlipBoard SOLARI' },
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
          'Labels follow the language you pick here. You can also shake your ' +
          'wrist to cycle to the next language.'
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
  { type: 'submit', defaultValue: 'Save settings' }
];
