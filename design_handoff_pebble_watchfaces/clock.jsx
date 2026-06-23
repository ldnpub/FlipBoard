// clock.jsx — shared watch state + formatting helpers for the Pebble Time 2 gallery
const STEP_GOAL = 10000;

const WEEKDAYS = {
  fr: ['DIM', 'LUN', 'MAR', 'MER', 'JEU', 'VEN', 'SAM'],
  en: ['SUN', 'MON', 'TUE', 'WED', 'THU', 'FRI', 'SAT'],
};
const LABELS = {
  steps: { fr: 'PAS', en: 'STEPS' },
  time:  { fr: 'HEURE', en: 'TIME' },
  date:  { fr: 'DATE', en: 'DATE' },
  batt:  { fr: 'BATTERIE', en: 'BATTERY' },
};

const pad2 = (n) => String(n).padStart(2, '0');

// red (0) -> amber -> green (goal+) ; works on dark screens
function stepsColor(steps, goal = STEP_GOAL, opts = {}) {
  const p = Math.max(0, Math.min(1, steps / goal));
  const hue = 4 + p * 130;            // 4=red -> 134=green
  const sat = (opts.sat ?? 90) - p * 6;
  const light = (opts.light ?? 56) + p * 4;
  return `hsl(${hue.toFixed(0)} ${sat.toFixed(0)}% ${light.toFixed(0)}%)`;
}

// Build the snapshot every variant reads from
function buildSnapshot(simMs, steps, battery, lang) {
  const d = new Date(simMs);
  const hh = pad2(d.getHours());
  const mm = pad2(d.getMinutes());
  const dd = pad2(d.getDate());
  const mo = pad2(d.getMonth() + 1);
  const colonOn = Math.floor(simMs / 500) % 2 === 0;
  const stepsInt = Math.max(0, Math.round(steps));
  return {
    hh, mm, colonOn,
    dateDM: `${dd}/${mo}`, dd, mo,
    year: d.getFullYear(),
    weekday: WEEKDAYS[lang][d.getDay()],
    weekdayFR: WEEKDAYS.fr[d.getDay()],
    weekdayEN: WEEKDAYS.en[d.getDay()],
    steps: stepsInt,
    stepsStr: String(stepsInt),
    stepsPad: String(stepsInt).padStart(5, '0'),
    goal: STEP_GOAL,
    progress: Math.max(0, Math.min(1, stepsInt / STEP_GOAL)),
    stepColor: stepsColor(stepsInt),
    battery: Math.round(battery),
    batteryStr: `${Math.round(battery)}%`,
    lang,
  };
}

// Drives time + steps; returns [snapshot, controls]
function useWatchEngine() {
  const [speed, setSpeed] = React.useState('fast'); // real | fast | turbo
  const [playing, setPlaying] = React.useState(true);
  const [lang, setLang] = React.useState('fr');
  const [manualSteps, setManualSteps] = React.useState(null); // null = auto
  const [battery, setBattery] = React.useState(86);

  const simRef = React.useRef(Date.now());
  const stepRef = React.useRef(3200);
  const [, force] = React.useReducer((x) => x + 1, 0);

  const speedRef = React.useRef(speed);
  const playRef = React.useRef(playing);
  const manualRef = React.useRef(manualSteps);
  speedRef.current = speed; playRef.current = playing; manualRef.current = manualSteps;

  React.useEffect(() => {
    const TICK = 200;
    const id = setInterval(() => {
      if (!playRef.current) return;
      const sp = speedRef.current;
      // advance sim time
      if (sp === 'real') simRef.current = Date.now();
      else simRef.current += TICK * (sp === 'turbo' ? 240 : 60);
      // advance steps (auto only)
      if (manualRef.current == null) {
        const factor = sp === 'turbo' ? 90 : sp === 'fast' ? 26 : 1.4;
        stepRef.current += Math.random() * factor + factor * 0.4;
        if (stepRef.current > STEP_GOAL + 1600) stepRef.current = Math.random() * 250;
      }
      force();
    }, TICK);
    return () => clearInterval(id);
  }, []);

  // colon is steady (no blink) — no heartbeat needed; real mode still ticks
  // the minute via the main interval below.

  const steps = manualSteps != null ? manualSteps : stepRef.current;
  const snap = buildSnapshot(simRef.current, steps, battery, lang);

  const controls = {
    speed, setSpeed, playing, setPlaying, lang, setLang,
    manualSteps, setManualSteps, battery, setBattery,
    bumpFlip: force,
  };
  return [snap, controls];
}

Object.assign(window, {
  STEP_GOAL, WEEKDAYS, LABELS, pad2, stepsColor, buildSnapshot, useWatchEngine,
});
