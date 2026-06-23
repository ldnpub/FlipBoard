// frame.jsx — Pebble Time 2 device chassis (steel body, 4 buttons, 200x228 e-paper)
function PebbleTime2({ children, screenClass = '', onTap }) {
  return (
    <div className="pt2">
      <div className="pt2-strap pt2-strap-top"></div>
      <div className="pt2-body">
        {/* left: back button */}
        <span className="pt2-btn pt2-btn-l pt2-back"></span>
        {/* right: up / select / down */}
        <span className="pt2-btn pt2-btn-r pt2-up"></span>
        <span className="pt2-btn pt2-btn-r pt2-sel"></span>
        <span className="pt2-btn pt2-btn-r pt2-dn"></span>

        <div className="pt2-glass">
          <div className={'pt2-screen ' + screenClass} onClick={onTap}>
            {children}
          </div>
        </div>
      </div>
      <div className="pt2-strap pt2-strap-bot"></div>
    </div>
  );
}

Object.assign(window, { PebbleTime2 });
