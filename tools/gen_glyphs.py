#!/usr/bin/env python3
"""Bake anti-aliased glyph sprite sheets for Pebble faces.
Each cell = ghost glyph ('8' for digits) + live glyph, composited with AA on the
face background, every pixel quantized to the 64-colour Pebble palette by its
DISPLAYED value (e-paper LUT) so the watch shows the closest possible colour."""
import json, sys
from PIL import Image, ImageDraw, ImageFont

LUT = {tuple(map(int, k.split(','))): tuple(v)
       for k, v in json.load(open('/tmp/epaper-lut.json')).items()}
PEBBLE = list(LUT.keys())

def quantize(target):
    # nearest by displayed value
    return min(PEBBLE, key=lambda c: sum((a-b)**2 for a, b in zip(LUT[c], target)))

def ramp_pick(alpha, ramp):
    # ramp: list of [threshold, [r,g,b]] sorted desc; first threshold <= alpha wins
    for thr, rgb in ramp:
        if alpha >= thr:
            return tuple(rgb)
    return None

def bake_sheet(ttf, size, glyphs, cell_w, cell_h, live_rgb, ghost_rgb, bg_rgb,
               out_png, ghost_char='8', y_off=0, live_ramp=None, ghost_ramp=None):
    font = ImageFont.truetype(ttf, size)
    sheet = Image.new('RGB', (cell_w * len(glyphs), cell_h), bg_rgb)
    for i, ch in enumerate(glyphs):
        # AA masks
        gm = Image.new('L', (cell_w, cell_h), 0)
        lm = Image.new('L', (cell_w, cell_h), 0)
        gch = ghost_char if ch.isdigit() else ch        # colon/hyphen ghost = itself
        for m, c in ((gm, gch), (lm, ch)):
            d = ImageDraw.Draw(m)
            bbox = d.textbbox((0, 0), c, font=font)
            x = (cell_w - (bbox[2] - bbox[0])) // 2 - bbox[0]
            y = (cell_h - (bbox[3] - bbox[1])) // 2 - bbox[1] + y_off
            d.text((x, y), c, font=font, fill=255)
        # composite: bg -> ghost -> live, then per-pixel quantize via LUT
        cell = Image.new('RGB', (cell_w, cell_h), bg_rgb)
        px, gp, lp = cell.load(), gm.load(), lm.load()
        for yy in range(cell_h):
            for xx in range(cell_w):
                ga, la = gp[xx, yy] / 255.0, lp[xx, yy] / 255.0
                if live_ramp is not None:
                    # hue-locked ramps: pick from a fixed warm family so the
                    # panel never drifts to pink/red on the AA edges
                    c = ramp_pick(la, live_ramp)
                    if c is None:
                        c = ramp_pick(ga, ghost_ramp) or tuple(bg_rgb)
                    px[xx, yy] = c
                else:
                    r, g, b = bg_rgb
                    r = r*(1-ga) + ghost_rgb[0]*ga; g = g*(1-ga) + ghost_rgb[1]*ga; b = b*(1-ga) + ghost_rgb[2]*ga
                    r = r*(1-la) + live_rgb[0]*la; g = g*(1-la) + live_rgb[1]*la; b = b*(1-la) + live_rgb[2]*la
                    px[xx, yy] = quantize((r, g, b))
        sheet.paste(cell, (i * cell_w, 0))
    sheet.save(out_png)
    print('baked', out_png, sheet.size)

if __name__ == '__main__':
    cfg = json.load(open(sys.argv[1]))
    for s in cfg['sheets']:
        bake_sheet(cfg['ttf'], s['size'], cfg['glyphs'], s['cell_w'], s['cell_h'],
                   tuple(cfg['live']), tuple(cfg['ghost']), tuple(cfg['bg']),
                   s['out'], cfg.get('ghost_char', '8'), s.get('y_off', 0),
                   cfg.get('live_ramp'), cfg.get('ghost_ramp'))
