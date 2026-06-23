# Handoff: Pebble Time 2 — Split-Flap / Dot-Matrix Watchfaces

## Overview
A family of **7 watchface designs** for the **Pebble Time 2**, all built around an
airport/train **split-flap "Solari" + dot-matrix departure-board** aesthetic. Each face
shows the same four pieces of data in the same vertical hierarchy:

1. **PAS / STEPS** (top) — colour-coded red→amber→green by progress toward a 10 000-step goal
2. **HEURE / TIME** (hero, largest) — `HH:MM`, 24-hour
3. **DATE** — `DD/MM`
4. **BATTERIE / BATTERY** (bottom) — `NN%`

The 7 faces differ in **typography + display mechanism** (split-flap tiles, LCD segments,
flip-clock, true LED dot-matrix, etc.). The user has reviewed all 7. Favourites called out
during design: **01, 02, 04, 06**, plus the LED face **07**. Nothing has been cut — ship
whichever the user selects, or all of them as a selectable set.

---

## About the Design Files
The files in this bundle are **design references created in HTML/React (JSX)** — interactive
prototypes that show the intended look and behaviour. **They are not production code to copy.**

The real target is a **Pebble smartwatch**, so the actual implementation is a **PebbleOS C app
built with the Pebble SDK** (watchface = C, using `Window`, `Layer`, `GBitmap`, `GFont`,
`text_layer`, `graphics_*`, `TickTimerService`, `BatteryStateService`, `HealthService`).
Recreate the *look* of these HTML mockups inside that C/SDK environment using its patterns —
do **not** attempt to run HTML/JS on the watch.

> If you (the developer) decide to target the modern **Rebble / PebbleOS** toolchain or a
> companion emulator, the same C SDK applies. Use the HTML only to read exact layout, colour,
> and proportion values.

## Fidelity
**High-fidelity.** Colours, proportions, type sizes, and layout are final and intentional.
Reproduce them faithfully. The one adaptation you MUST make: the HTML is authored on a
**300×342 design canvas (1.5× scale)**; the real screen is **200×228**. Multiply every px
value below by **0.667** to get device pixels, or just re-derive from the proportions.

---

## Target device — Pebble Time 2
- **Screen: 200 × 228 px**, color e-paper (64 colors), rectangular.
- Always-on display; **minimize redraws to save battery** (see "Battery / redraw" below).
- Buttons: Back (left); Up / Select / Down (right). Watchfaces don't need button handlers
  unless you add a face-switcher (recommended: Up/Down cycles faces, persisted via
  `persist_write_int`).
- Fonts must be embedded as **resources** (TTF→Pebble font via the SDK, or hand-built bitmap
  fonts). All typefaces used here are open-licensed (see "Assets / fonts").

---

## Global data + logic

### Steps colour ramp (the signature interaction)
Steps value drives a continuous **red→amber→green** colour. Reference implementation
(`clock.jsx → stepsColor`), in HSL:

```
p     = clamp(steps / 10000, 0, 1)
hue   = 4 + p*130       // 4° red  → 134° green
sat   = 90 - p*6        // ~90% → ~84%
light = 56 + p*4        // ~56% → ~60%
color = hsl(hue, sat%, light%)
```

On the 64-colour Pebble you can't show a smooth gradient, so **quantise to the nearest
GColor**. Suggested 5-stop ramp by progress:

| Steps        | Meaning            | Suggested GColor        | approx hex |
|--------------|--------------------|-------------------------|-----------|
| 0–1 999      | very low (red)     | `GColorRed`             | `#FF0000` |
| 2 000–3 999  | low (orange)       | `GColorOrange`          | `#FF5500` (use `#FFAA00` if too harsh) |
| 4 000–6 499  | mid (amber/yellow) | `GColorChromeYellow` / `GColorYellow` | `#FFAA00` / `#FFFF00` |
| 6 500–8 999  | good (yellow-green)| `GColorBrightGreen` / `GColorKellyGreen` | `#55FF00` / `#00AA00` |
| 9 000+       | goal (green)       | `GColorGreen`           | `#00FF00` |

(Smooth HSL is the design intent; the 5-stop quantisation is the device-accurate
approximation. Either is acceptable — match the spirit: clearly red when low, clearly green
near goal.)

### Time / formatting
- 24-hour `HH:MM`, zero-padded. Colon is **steady (never blinks)** — see battery note.
- Date `DD/MM`, zero-padded.
- Battery `NN%` (integer).
- Weekday abbreviations (used as status text on faces 05/06):
  - FR: `DIM LUN MAR MER JEU VEN SAM`
  - EN: `SUN MON TUE WED THU FRI SAT`

### Bilingual labels (FR / EN)
Every face must support **both languages** (user requirement). Labels switch as a set;
expose a setting (config page or buried button) and persist it.

| key   | FR        | EN       |
|-------|-----------|----------|
| steps | `PAS`     | `STEPS`  |
| time  | `HEURE`   | `TIME`   |
| date  | `DATE`    | `DATE`   |
| batt  | `BATTERIE`/`BATT` | `BATTERY`/`BATT` |

### Battery / redraw (important — user explicitly asked)
- **No blinking anywhere.** The colon is steady; status dots/cursors are steady. Earlier
  prototypes blinked the colon every 500 ms — that was removed to avoid needless e-paper
  refreshes.
- Subscribe to `TickTimerService` at **MINUTE** resolution (not seconds). Only redraw the
  changed layer. Steps/battery via their services' callbacks, not polling.
- The split-flap / dot "refresh sweep" animation is **optional eye-candy** — on real hardware,
  prefer instant redraw (or a very short, one-shot animation only when a digit changes) to
  protect battery. The prototype runs animations only because it's a screen demo.

---

## Layout (shared skeleton)
Vertical stack, full screen, generous side padding. On the 300×342 canvas the padding is
~16 px (≈ 11 px on device). Top→bottom: **PAS row → TIME (hero, centered/又 right) → DATE row
→ BATTERIE row**, distributed with space-between. Labels are small mono caps with wide
letter-spacing (~0.2em); values are large in the face's display font. On rows, **label is
left-aligned, value right-aligned to a shared right edge**.

Device-pixel guidance (200×228): hero time ≈ **48–56 px tall**; secondary values ≈ 28–34 px;
labels ≈ 8–9 px mono. Never smaller than ~8 px on device for labels.

---

## The 7 faces

> px sizes below are **design-canvas px (300-wide)**. Multiply by 0.667 for device px.

### 01 · DOT-MATRIX  (font: **Doto**, var weight 800)
- Background `#0C0D0F` with a faint radial dot texture (7px grid, ~55% opacity white dots).
- Text `#F4F2EC` (near-white) for time/date/battery; **steps in the ramp colour**.
- Hero clock 90px. Steps 50px. Date/battery 46px. Labels mono 12px `#7A7C83`.
- **Colon = two square dots** (NOT a font colon — Doto's `:` renders as plus-shapes, which we
  deliberately avoided). Draw two ~0.135em squares, vertically centered, on the digit grid.
- This is the face most faithful to the user's `reference/header.png`.

### 02 · TABLEAU D'AÉROPORT  (font: **Silkscreen**)
- Background `#070809`. True **split-flap tiles**: each character sits on a dark rounded tile
  (`#101216`, radius 2px) with a horizontal seam at the vertical midpoint (top half darker,
  bottom half lighter highlight). Tiles separated by ~3px.
- Header row: `DÉPARTS`/`DEPARTURES` + a steady amber status dot `●` `#C59B3E`.
- Rows: label (mono 12px `#7E8087`) + value tiles. Time row centered, 66px. Other values 34px.
- Steps padded to 5 digits (`03227`), in ramp colour.

### 03 · RÉVEIL FLIP  (font: **Oswald**, weight 600)  — the only LIGHT-tile face
- Screen `#14161B`. Big **ivory flip-clock tiles** `#ECE7DA` with digits `#23211C`, rounded
  9px, drop shadow + top highlight, horizontal fold seam.
- Hero `HH : MM` ~78px on ivory tiles; the colon is two round dots `#E7E1D4` between the
  tile pairs.
- Steps (top, ramp colour, 46px), DATE + BATTERIE bottom row (Oswald 42px, `%` rendered
  smaller in mono `#B9B3A6`).

### 04 · TERMINAL  (font: **VT323**)
- Background `#0C0D0F` (shared dark). Greenish terminal palette: text `#E1E5DC`,
  dim `#7F8B7A`, accent green `#9BD27F`.
- Top status line: `▸ SYS·OK   24H   ▮` (cursor steady, not blinking).
- Steps block: `PAS  / 10000` label line, big steps `03200` (48px) in ramp colour, plus a
  **progress bar** (track `#181D16`, fill = ramp colour, width = progress%).
- Hero clock 102px. Footer: DATE + BATTERIE (VT323 44px).

### 05 · DÉPARTS GARE  (font: **DSEG7 Classic**, 7-segment) — amber platform board
- Background `#09090A`. Everything amber `#F6A91E`; **ghost segments** behind each glyph
  (same text at ~13% opacity) for the authentic LCD look.
- Header: `▸ DÉPARTS` + weekday. Hero 7-seg clock 70px.
- **PAS on its own wide line** (38px, ramp colour) — made prominent for readability on the
  small screen. Then DATE + BATTERIE as a 2-up row (34px). `%` smaller in mono.

### 06 · BORNE DÉPARTS  (font: **DSEG14 Classic**, 14-segment) — bilingual board
- Background `#0A0C0E`. Header bar `#101317`: `PEBBLE·DÉPARTS` + small clock + steady dot.
- 4 board rows, each: bilingual label (`PAS`/`STEPS` stacked), 14-seg value, right-side
  **status** word. Steps row big (35px) + `%` status in ramp colour; TIME row status
  `À L'HEURE`/`ON TIME` green `#5EC98A`; DATE status = weekday; BATT status `OK` or
  `FAIBLE`/`LOW` red `#E8553E` when <20%.

### 07 · TABLEAU LED  (custom 5×7 dot-matrix, canvas) — authentic departure board
- The literal "lit dots on a visible unlit-dot grid" look from
  `reference/airport-board-reference.jpg`.
- **Every glyph is a 5×7 dot cell** (1-cell gap between glyphs). **Unlit dots** are drawn in
  grey `rgba(238,236,228,0.16)`; **lit dots** in the value colour. Dot radius ≈ 0.37 × cell.
- Background `#060607` with a subtle vignette. Time `#F4F2EC` (white), DATE amber `#F6A91E`,
  BATTERY white, **STEPS in ramp colour**. Slight LED glow via a small drop-shadow.
- Bitmap font for digits + `: / - %` is fully defined in `dotmatrix.jsx` (`DM_FONT`). On
  Pebble, implement this as direct `graphics_fill_circle` calls over a 5×7 bit table, or bake
  PNG sprites — the 5×7 patterns are the source of truth.
- This is the best candidate for a pixel-accurate Pebble build because it's resolution-defined,
  not font-dependent.

---

## Design tokens

### Colors
| Token | Hex | Use |
|---|---|---|
| ink / screen dark | `#0C0D0F` / `#070809` / `#060607` / `#09090A` | face backgrounds |
| near-white text | `#F4F2EC` / `#E7E1D4` / `#E1E5DC` | time/date/battery |
| amber | `#F6A91E` | DSEG faces, LED date, status dot `#C59B3E` |
| ivory tile | `#ECE7DA` (digit `#23211C`) | face 03 flip tiles |
| terminal green | `#9BD27F` / status `#5EC98A` | face 04/06 |
| alert red | `#E8553E` | low-battery status |
| steps ramp | `hsl(4→134, 90→84%, 56→60%)` | steps value on every face |
| label grey | `#7A7C83` / `#7E8087` / `#6F7178` | mono labels |
| LED ghost dot | `rgba(238,236,228,0.16)` | face 07 unlit grid |

### Typography (all open-licensed — see Assets)
- **Doto** (variable) — face 01
- **Silkscreen** — face 02
- **Oswald** (variable) — face 03
- **VT323** — face 04
- **DSEG7 Classic** (bold) — face 05
- **DSEG14 Classic** (bold) — face 06
- **Custom 5×7 dot-matrix** (defined in `dotmatrix.jsx`) — face 07
- **Mono** (DM Mono / system mono) — all small labels, wide tracking ~0.2em, UPPERCASE

### Spacing / radius
- Screen padding ≈ 16px canvas (≈11px device).
- Split-flap tile radius 2px (02), flip-tile radius 9px (03).
- Row gap: space-between within the vertical stack.

---

## Assets / fonts (licensing)
All display fonts are free for commercial use:
- **Doto, Silkscreen, VT323, Oswald** — SIL Open Font License (OFL). Bundle the TTFs as Pebble
  font resources.
- **DSEG7 / DSEG14 Classic** — OFL (github.com/keshikan/DSEG).
- **5×7 dot-matrix** — original, defined in this bundle (`dotmatrix.jsx` `DM_FONT`). No license.
- **`reference/header.png`** — user's own theme reference.
- **`reference/airport-board-reference.jpg`** — Freepik (macrovector). Inspiration only; **do
  not embed the JPG in the app**. If any derived art ships, credit *"Designed by macrovector /
  Freepik"* (see `reference/airport-board-reference-LICENSE.txt`).

In the prototype these load as web fonts (Google Fonts / jsDelivr). For Pebble, convert/bundle
each as an embedded resource and pick the nearest available weight.

---

## Files in this bundle
- `Watchfaces.html` — entry point; loads React + Babel and all the JSX below. Open in a browser
  to see all 7 faces live, with a control bar (demo speed, play/pause, steps slider 0–12 000,
  battery slider, FR/EN toggle). All face CSS (backgrounds, tiles, colours, sizes) lives in the
  `<style>` block of this file — **this is the source of truth for visual values.**
- `clock.jsx` — shared state engine: time simulation, steps auto-walk, `stepsColor()` ramp,
  formatting, FR/EN label + weekday tables (`LABELS`, `WEEKDAYS`), `STEP_GOAL = 10000`.
- `flap.jsx` — split-flap mechanism (`Flap`, `FlapText`) used by faces 01/02/04/06.
- `dotmatrix.jsx` — 5×7 bitmap font (`DM_FONT`) + canvas `DotMatrix` renderer (face 07).
- `frame.jsx` — Pebble Time 2 device chassis mock (bezel + buttons) — presentation only,
  **not** part of the watchface UI.
- `variants.jsx` — the 7 face components (`V01`–`V07`) and the `VARIANTS` registry (id, label,
  screen-background class, component). Read each `VNN` for exact element/structure per face.
- `app.jsx` — gallery + control bar (demo harness only; not shipped to the watch).
- `reference/` — the user's `header.png`, the airport-board inspiration JPG + its license.

## Suggested build order (Pebble C)
1. Scaffold a watchface; subscribe to minute ticks, `HealthService` (steps),
   `BatteryStateService`. Persist FR/EN + selected-face index.
2. Implement the **shared layout** (4 stacked zones) and the **steps→GColor** ramp once.
3. Build **face 07 (LED 5×7)** first — it's resolution-defined and the cleanest port (circles
   on a grid), and it's a user favourite direction.
4. Add font-based faces (01 Doto, 02 Silkscreen, 04 VT323, 06 DSEG14, 05 DSEG7, 03 Oswald),
   bundling each font resource and matching sizes/colours from `Watchfaces.html`.
5. Optional: Up/Down buttons cycle faces; keep all redraws minute-gated and blink-free.
