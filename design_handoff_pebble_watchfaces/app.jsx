// app.jsx — control bar + gallery of the 6 watchfaces
const { useState } = React;

function Segmented({ value, onChange, options }) {
  return (
    <div className="seg-ctl">
      {options.map((o) => (
        <button key={o.v} className={'seg-btn' + (value === o.v ? ' on' : '')}
          onClick={() => onChange(o.v)}>{o.label}</button>
      ))}
    </div>
  );
}

function ControlBar({ c, snap }) {
  const stepVal = c.manualSteps != null ? c.manualSteps : snap.steps;
  return (
    <header className="bar">
      <div className="bar-id">
        <div className="bar-title">PEBBLE&nbsp;TIME&nbsp;2 — <b>SPLIT-FLAP</b></div>
        <div className="bar-sub">7 watchfaces · 200×228 · battement mécanique · pas rouge→vert</div>
      </div>
      <div className="bar-ctls">
        <div className="ctl">
          <label>VITESSE</label>
          <Segmented value={c.speed} onChange={c.setSpeed}
            options={[{ v: 'real', label: 'Réel' }, { v: 'fast', label: 'Rapide' }, { v: 'turbo', label: 'Turbo' }]} />
        </div>
        <button className={'play' + (c.playing ? ' on' : '')} onClick={() => c.setPlaying(!c.playing)}>
          {c.playing ? '❚❚ Pause' : '▶ Lecture'}
        </button>
        <div className="ctl ctl-slider">
          <label>PAS <b style={{ color: snap.stepColor }}>{stepVal}</b>{c.manualSteps != null && <span className="man">manuel</span>}</label>
          <div className="slider-row">
            <input type="range" min="0" max="12000" step="50" value={stepVal}
              onChange={(e) => c.setManualSteps(+e.target.value)} />
            <button className="mini" onClick={() => c.setManualSteps(null)} disabled={c.manualSteps == null}>Auto</button>
          </div>
        </div>
        <div className="ctl ctl-slider">
          <label>BATTERIE <b>{snap.battery}%</b></label>
          <input type="range" min="0" max="100" step="1" value={c.battery}
            onChange={(e) => c.setBattery(+e.target.value)} />
        </div>
        <div className="ctl">
          <label>LANGUE</label>
          <Segmented value={c.lang} onChange={c.setLang}
            options={[{ v: 'fr', label: 'FR' }, { v: 'en', label: 'EN' }]} />
        </div>
      </div>
    </header>
  );
}

function Card({ v, t }) {
  const { Comp } = v;
  return (
    <div className="card">
      <div className="card-head">
        <span className="card-n">{v.n}</span>
        <div className="card-meta">
          <div className="card-label">{v.label}</div>
          <div className="card-sub">{v.sub}</div>
        </div>
      </div>
      <div className="card-stage">
        <PebbleTime2 screenClass={v.screenClass}>
          <Comp t={t} />
        </PebbleTime2>
      </div>
    </div>
  );
}

function App() {
  const [snap, c] = useWatchEngine();
  return (
    <div className="app">
      <ControlBar c={c} snap={snap} />
      <main className="grid">
        {VARIANTS.map((v) => <Card key={v.id} v={v} t={snap} />)}
      </main>
      <footer className="page-foot">
        Astuce — passe en <b>Turbo</b> pour voir les volets s'enchaîner, ou tire le curseur <b>PAS</b> de 0 à 10 000 pour balayer le dégradé rouge→vert.
      </footer>
    </div>
  );
}

ReactDOM.createRoot(document.getElementById('root')).render(<App />);
