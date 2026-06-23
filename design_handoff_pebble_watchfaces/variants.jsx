// variants.jsx — the 6 Pebble Time 2 watchface designs
// Each component receives `t` = snapshot from useWatchEngine.

// shared clock — hh : mm. Colon is a plain glyph (no flap) so it sits cleanly
// on the font's own dot grid with no fold seam.
function ClockFlap({ t, className = '', dur = 240 }) {
  return (
    <span className={'clockflap ' + className}>
      <FlapText text={t.hh} dur={dur} />
      <span className="cf-colon">:</span>
      <FlapText text={t.mm} dur={dur} />
    </span>
  );
}

function LabelVal({ lbl, children, cls = '' }) {
  return (
    <div className={'lv ' + cls}>
      <span className="lv-lbl">{lbl}</span>
      <span className="lv-val">{children}</span>
    </div>
  );
}

/* ───────────────────────── 01 · DOT-MATRIX (Doto) ───────────────────────── */
function V01({ t }) {
  return (
    <div className="v01">
      <div className="v01-row v01-row-steps">
        <span className="v01-lbl">{LABELS.steps[t.lang]}</span>
        <FlapText className="v01-val" style={{ color: t.stepColor }} text={t.stepsStr} />
      </div>
      <div className="v01-hero">
        <ClockFlap t={t} className="v01-clock" />
      </div>
      <div className="v01-row">
        <span className="v01-lbl">{LABELS.date[t.lang]}</span>
        <FlapText className="v01-sub" text={t.dateDM} />
      </div>
      <div className="v01-row">
        <span className="v01-lbl">{LABELS.batt[t.lang]}</span>
        <FlapText className="v01-sub" text={t.batteryStr} />
      </div>
    </div>
  );
}

/* ─────────────────────── 02 · TABLEAU D'AÉROPORT (Silkscreen) ─────────────── */
function V02({ t }) {
  return (
    <div className="v02">
      <div className="v02-head">
        <span>{t.lang === 'fr' ? 'DÉPARTS' : 'DEPARTURES'}</span>
        <span className="v02-blink">●</span>
      </div>
      <div className="v02-rows">
        <div className="v02-row">
          <span className="v02-lbl">{LABELS.steps[t.lang]}</span>
          <FlapText className="v02-sm" flapClass="tile" style={{ color: t.stepColor }} text={t.stepsPad} />
        </div>
        <div className="v02-row v02-time">
          <FlapText className="v02-big" flapClass="tile" text={`${t.hh}:${t.mm}`} />
        </div>
        <div className="v02-row">
          <span className="v02-lbl">{LABELS.date[t.lang]}</span>
          <FlapText className="v02-sm" flapClass="tile" text={t.dateDM} />
        </div>
        <div className="v02-row">
          <span className="v02-lbl">{LABELS.batt[t.lang]}</span>
          <FlapText className="v02-sm" flapClass="tile" text={t.batteryStr} />
        </div>
      </div>
    </div>
  );
}

/* ───────────────────────────── 03 · LCD (DSEG7) ──────────────────────────── */
function LcdRow({ value, className = '', color }) {
  const ghost = String(value).replace(/[0-9]/g, '8');
  return (
    <span className={'lcdrow ' + className}>
      <FlapText className="lcd-ghost" text={ghost} />
      <FlapText className="lcd-live" style={color ? { color } : undefined} text={value} />
    </span>
  );
}
function V03({ t }) {
  return (
    <div className="v03">
      <div className="v03-top">
        <span className="v03-lbl">{LABELS.steps[t.lang]}</span>
        <FlapText className="v03-steps" style={{ color: t.stepColor }} text={t.stepsStr} dur={280} />
      </div>
      <div className="v03-hero">
        <FlapText className="v03-clock" flapClass="ctile" text={t.hh} dur={300} />
        <span className="v03-colon"><b></b><b></b></span>
        <FlapText className="v03-clock" flapClass="ctile" text={t.mm} dur={300} />
      </div>
      <div className="v03-bot">
        <div className="v03-cell">
          <span className="v03-lbl">{LABELS.date[t.lang]}</span>
          <FlapText className="v03-sm" text={t.dateDM} dur={280} />
        </div>
        <div className="v03-cell">
          <span className="v03-lbl">{LABELS.batt[t.lang]}</span>
          <span className="v03-batt"><FlapText className="v03-sm" text={String(t.battery)} dur={280} /><i>%</i></span>
        </div>
      </div>
    </div>
  );
}

/* ─────────────────────── 04 · TERMINAL / AÉROSPATIAL (VT323) ──────────────── */
function V04({ t }) {
  return (
    <div className="v04">
      <div className="v04-status">
        <span>▸ SYS·OK</span><span>24H</span><span className="v04-cur">▮</span>
      </div>
      <div className="v04-steps-block">
        <div className="v04-line">
          <span className="v04-k">{LABELS.steps[t.lang]}</span>
          <span className="v04-goal">/ {t.goal}</span>
        </div>
        <FlapText className="v04-steps" style={{ color: t.stepColor }} text={t.stepsPad} />
        <div className="v04-bar"><span style={{ width: (t.progress * 100).toFixed(1) + '%', background: t.stepColor }}></span></div>
      </div>
      <div className="v04-hero"><ClockFlap t={t} className="v04-clock" /></div>
      <div className="v04-foot">
        <div className="v04-fcell">
          <span className="v04-k">{LABELS.date[t.lang]}</span>
          <FlapText className="v04-fv" text={t.dateDM} />
        </div>
        <div className="v04-fcell">
          <span className="v04-k">{LABELS.batt[t.lang]}</span>
          <FlapText className="v04-fv" text={t.batteryStr} />
        </div>
      </div>
    </div>
  );
}

/* ──────────────────────────── 05 · DÉPARTS GARE (DSEG7 ambre) ─────────────── */
function V05({ t }) {
  return (
    <div className="v05">
      <div className="v05-head">
        <span>{t.lang === 'fr' ? '▸ DÉPARTS' : '▸ DEPARTURES'}</span>
        <span className="v05-day">{t.weekday}</span>
      </div>
      <div className="v05-hero">
        <LcdRow className="v05-clock" value={`${t.hh}:${t.mm}`} />
      </div>
      <div className="v05-steps">
        <span className="v05-k">{LABELS.steps[t.lang]}</span>
        <LcdRow className="v05-sv" value={t.stepsStr} color={t.stepColor} />
      </div>
      <div className="v05-foot">
        <div className="v05-cell">
          <span className="v05-k">{LABELS.date[t.lang]}</span>
          <LcdRow className="v05-fv" value={t.dateDM.replace('/', '-')} />
        </div>
        <div className="v05-cell">
          <span className="v05-k">{LABELS.batt[t.lang]}</span>
          <span className="v05-bv"><LcdRow className="v05-fv" value={String(t.battery)} /><i>%</i></span>
        </div>
      </div>
    </div>
  );
}

/* ──────────────────── 06 · BORNE DÉPARTS / MISSION (DSEG14) ───────────────── */
function BoardRow({ fr, en, value, suffix, status, statusColor, valueColor, big }) {
  return (
    <div className={'br' + (big ? ' br-big' : '')}>
      <span className="br-lbl"><b>{fr}</b><i>{en}</i></span>
      <span className="br-valwrap">
        <FlapText className="br-val" flapClass="seg" style={valueColor ? { color: valueColor } : undefined} text={value} />
        {suffix && <span className="br-suf">{suffix}</span>}
      </span>
      <span className="br-status" style={statusColor ? { color: statusColor } : undefined}>{status}</span>
    </div>
  );
}
function V06({ t }) {
  const pct = Math.round(t.progress * 100);
  return (
    <div className="v06">
      <div className="v06-head">
        <span className="v06-brand">PEBBLE<i>·</i>DÉPARTS</span>
        <span className="v06-clock">{t.hh}:{t.mm}<b>●</b></span>
      </div>
      <div className="v06-rows">
        <BoardRow fr="PAS" en="STEPS" value={t.stepsPad} valueColor={t.stepColor}
          status={`${pct}%`} statusColor={t.stepColor} big />
        <BoardRow fr="HEURE" en="TIME" value={`${t.hh}:${t.mm}`}
          status={t.lang === 'fr' ? "À L'HEURE" : 'ON TIME'} statusColor="#5ec98a" />
        <BoardRow fr="DATE" en="DATE" value={t.dateDM} status={t.weekday} />
        <BoardRow fr="BATT" en="BATT" value={String(t.battery)} suffix="%"
          status={t.battery < 20 ? (t.lang === 'fr' ? 'FAIBLE' : 'LOW') : 'OK'}
          statusColor={t.battery < 20 ? '#e8553e' : undefined} />
      </div>
    </div>
  );
}

/* ────────────────── 07 · TABLEAU LED (vraie matrice de points) ────────────── */
function V07({ t }) {
  const amber = '#f6a91e', white = '#f4f2ec';
  return (
    <div className="v07">
      <div className="v07-row">
        <span className="v07-lbl">{LABELS.steps[t.lang]}</span>
        <DotMatrix text={t.stepsStr} color={t.stepColor} pitch={7} />
      </div>
      <div className="v07-hero">
        <DotMatrix text={`${t.hh}:${t.mm}`} color={white} pitch={8.6} />
      </div>
      <div className="v07-row">
        <span className="v07-lbl">{LABELS.date[t.lang]}</span>
        <DotMatrix text={t.dateDM} color={amber} pitch={6.6} />
      </div>
      <div className="v07-row">
        <span className="v07-lbl">{LABELS.batt[t.lang]}</span>
        <DotMatrix text={t.batteryStr} color={white} pitch={6.6} />
      </div>
    </div>
  );
}

const VARIANTS = [
  { id: 'dotmatrix', n: '01', label: 'DOT-MATRIX', sub: 'Doto · fidèle au header, points e-paper', screenClass: 'sc-dark', Comp: V01 },
  { id: 'airport', n: '02', label: "TABLEAU D'AÉROPORT", sub: 'Silkscreen · volets Solari mécaniques', screenClass: 'sc-board', Comp: V02 },
  { id: 'flip', n: '03', label: 'RÉVEIL FLIP', sub: 'Oswald · tuiles ivoire, pli mécanique', screenClass: 'sc-flip', Comp: V03 },
  { id: 'terminal', n: '04', label: 'TERMINAL', sub: 'VT323 · console aérospatiale minimale', screenClass: 'sc-dark', Comp: V04 },
  { id: 'station', n: '05', label: 'DÉPARTS GARE', sub: 'DSEG7 ambre · afficheur de quai', screenClass: 'sc-amber', Comp: V05 },
  { id: 'board', n: '06', label: 'BORNE DÉPARTS', sub: 'DSEG14 · grille bilingue type aéroport', screenClass: 'sc-board2', Comp: V06 },
  { id: 'led', n: '07', label: 'TABLEAU LED', sub: 'Matrice 5×7 · points allumés/éteints', screenClass: 'sc-led', Comp: V07 },
];

Object.assign(window, { ClockFlap, LcdRow, V01, V02, V03, V04, V05, V06, V07, VARIANTS });
