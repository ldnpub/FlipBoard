// dotmatrix.jsx — authentic LED split-flap board renderer.
// Every character is a 5x7 cell; UNLIT dots are drawn faint (the "ghost grid"),
// LIT dots bright. A short top-down reveal plays when a value changes.

const DM_FONT = {
  '0': ['01110','10001','10011','10101','11001','10001','01110'],
  '1': ['00100','01100','00100','00100','00100','00100','01110'],
  '2': ['01110','10001','00001','00110','01000','10000','11111'],
  '3': ['11110','00001','00001','01110','00001','00001','11110'],
  '4': ['00010','00110','01010','10010','11111','00010','00010'],
  '5': ['11111','10000','11110','00001','00001','10001','01110'],
  '6': ['00110','01000','10000','11110','10001','10001','01110'],
  '7': ['11111','00001','00010','00100','01000','01000','01000'],
  '8': ['01110','10001','10001','01110','10001','10001','01110'],
  '9': ['01110','10001','10001','01111','00001','00010','01100'],
  ':': ['00000','00000','00100','00000','00100','00000','00000'],
  '/': ['00001','00001','00010','00100','01000','10000','10000'],
  '-': ['00000','00000','00000','11111','00000','00000','00000'],
  '%': ['11001','11010','00100','01011','10011','00000','00000'],
  ' ': ['00000','00000','00000','00000','00000','00000','00000'],
};

const COLS = 5, ROWS = 7, GAP = 1; // 1-cell gap between chars

// React component. Props: text, color, ghost, pitch (px per cell), className, style
function DotMatrix({ text, color = '#f6a91e', ghost = 'rgba(238,236,228,0.16)', pitch = 9, className = '', style }) {
  const ref = React.useRef(null);

  React.useEffect(() => {
    const canvas = ref.current;
    if (!canvas) return;
    const chars = String(text).split('');
    const advance = COLS + GAP;
    const cells = chars.length * advance;
    const dpr = Math.min(window.devicePixelRatio || 1, 2);
    const W = cells * pitch, H = ROWS * pitch;
    const Wd = Math.round(W * dpr), Hd = Math.round(H * dpr);
    // resize ONLY when dimensions change (resizing clears the canvas)
    if (canvas.width !== Wd || canvas.height !== Hd) {
      canvas.width = Wd; canvas.height = Hd;
      canvas.style.width = W + 'px'; canvas.style.height = H + 'px';
    }
    const ctx = canvas.getContext('2d');
    ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
    const r = pitch * 0.37;

    ctx.clearRect(0, 0, W, H);
    let cx = 0;
    for (const c of chars) {
      const bm = DM_FONT[c] || DM_FONT[' '];
      for (let row = 0; row < ROWS; row++) {
        for (let col = 0; col < COLS; col++) {
          const bit = bm[row][col] === '1';
          const x = (cx + col + 0.5) * pitch;
          const y = (row + 0.5) * pitch;
          ctx.beginPath();
          ctx.arc(x, y, bit ? r : r * 0.9, 0, Math.PI * 2);
          ctx.fillStyle = bit ? color : ghost;
          ctx.fill();
        }
      }
      cx += advance;
    }
  }, [text, color, ghost, pitch]);

  return React.createElement('canvas', { ref, className: 'dm ' + className, style });
}

Object.assign(window, { DotMatrix, DM_FONT });
