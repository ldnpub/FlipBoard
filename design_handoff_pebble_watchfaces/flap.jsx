// flap.jsx — mechanical split-flap (Solari board) character + text row
// A cell shows `current`; when `ch` changes it flips: old top folds down
// revealing new top, then new bottom drops over old bottom.

function Flap({ ch, dur = 240 }) {
  const [current, setCurrent] = React.useState(ch);
  const [target, setTarget] = React.useState(null);

  React.useEffect(() => {
    if (ch !== current && target === null) setTarget(ch);
  }, [ch, current, target]);

  const flipping = target !== null;
  const onEnd = () => { setCurrent(target); setTarget(null); };

  const half = (which, c) => (
    React.createElement('span', { className: 'fh fh-' + which },
      React.createElement('span', { className: 'fch' }, c))
  );

  return React.createElement('span', { className: 'flap' + (flipping ? ' is-flipping' : '') },
    half('top', flipping ? target : current),   // static top = incoming
    half('bot', current),                       // static bottom = outgoing
    flipping && React.createElement('span', {
      className: 'leaf leaf-top',
      style: { animationDuration: (dur / 2) + 'ms' },
    }, half('top', current)),
    flipping && React.createElement('span', {
      className: 'leaf leaf-bot',
      style: { animationDuration: (dur / 2) + 'ms', animationDelay: (dur / 2) + 'ms' },
      onAnimationEnd: onEnd,
    }, half('bot', target)),
  );
}

// FlapText: render a string as a row of flaps. Spaces -> spacer.
function FlapText({ text, className = '', style, dur = 240, flapClass = '' }) {
  const chars = String(text).split('');
  return React.createElement('span', { className: 'flapline ' + className, style },
    chars.map((c, i) =>
      c === ' '
        ? React.createElement('span', { className: 'flap flap-space', key: i })
        : React.createElement('span', { className: 'flapwrap ' + flapClass, key: i },
            React.createElement(Flap, { ch: c, dur }))
    )
  );
}

Object.assign(window, { Flap, FlapText });
