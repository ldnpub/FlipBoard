// FlipBoard · STIPPLE (face 01) — Doto dot-grain on a faint stippled e-paper
// field. Layout per design_handoff variants.jsx V01:
//   STEPS (ramp colour) → HERO HH:MM (Doto, square colon) → DATE → BATTERY.
// Doto's own ':' renders as a plus shape, so the colon is drawn as two squares.

#include "flipboard.h"

#define PAD 11
#define COL_INK    GColorFromRGB(0x0C, 0x0D, 0x0F)   // screen
#define COL_TEXT   GColorFromRGB(0xF4, 0xF2, 0xEC)   // near-white
#define COL_LABEL  GColorLightGray                    // label grey #7a7c83 — LightGray reads closest on panel

static GFont s_sub;    // Doto 30 — steps / date / battery

// Hero digits: pre-baked anti-aliased Doto 60 sprites (tools/gen_glyphs.py),
// white with grey AA edges — sheet order "0123456789".
#define GCW 36
#define GCH 64
static GBitmap *s_doto, *s_dg[10];

static void setup(void) {
  s_sub  = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DOTO_30));
  s_doto = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_DOTO60);
  for (int i = 0; i < 10; i++)
    s_dg[i] = gbitmap_create_as_sub_bitmap(s_doto, GRect(i * GCW, 0, GCW, GCH));
}
// Split-flap fold illusion for one sprite cell (f = st->anim, 5..0): the old
// glyph's top half folds away, then the new glyph unrolls from the seam.
static void flap_cell(GContext *ctx, GBitmap *oldb, GBitmap *newb, GRect r, int f, GColor cover) {
  const int hh = r.size.h / 2;
  graphics_context_set_compositing_mode(ctx, GCompOpAssign);
  if (f >= 5) {
    graphics_draw_bitmap_in_rect(ctx, oldb, r);
  } else if (f >= 3) {
    graphics_draw_bitmap_in_rect(ctx, oldb, r);
    graphics_context_set_fill_color(ctx, cover);
    graphics_fill_rect(ctx, GRect(r.origin.x, r.origin.y, r.size.w, f == 4 ? hh / 2 : hh), 0, GCornerNone);
  } else {
    graphics_draw_bitmap_in_rect(ctx, newb, r);
    if (f > 0) {
      graphics_context_set_fill_color(ctx, cover);
      graphics_fill_rect(ctx, GRect(r.origin.x, r.origin.y + hh, r.size.w, f == 2 ? hh : hh / 2), 0, GCornerNone);
    }
  }
}


// One stacked row: mono label left, Doto value right (shared right edge).
// `x`/`w` come from the live layer bounds — no hard-coded screen size.
static void row(GContext *ctx, const char *lbl, const char *val, GColor vc,
                int x, int w, int y) {
  graphics_context_set_text_color(ctx, COL_LABEL);
  graphics_draw_text(ctx, lbl, fonts_get_system_font(FONT_KEY_GOTHIC_14),
                     GRect(x + PAD, y + 12, w - 2*PAD, 18),
                     GTextOverflowModeFill, GTextAlignmentLeft, NULL);
  graphics_context_set_text_color(ctx, vc);
  graphics_draw_text(ctx, val, s_sub,
                     GRect(x + PAD, y, w - 2*PAD, 42),
                     GTextOverflowModeFill, GTextAlignmentRight, NULL);
}

static void render(GContext *ctx, const FlipState *st, struct tm *now, GRect area) {
  // Everything derives from `area` (the live unobstructed bounds) — no screen
  // size is hard-coded, so the face is resolution-correct and Quick View aware.
  const int left = area.origin.x, top = area.origin.y;
  const int w = area.size.w, h = area.size.h;
  const int bot = top + h, cx = left + w / 2;

  // Plain ink field — design vignette is below the 64-colour quantization step;
  // dithering it reads as noise on the panel (tested 2026-06-11, reverted).
  graphics_context_set_fill_color(ctx, COL_INK);
  graphics_fill_rect(ctx, area, 0, GCornerNone);
  (void)bot;

  char hh[4], mm[4], steps[16], date[24], batt[16];
  snprintf(hh, sizeof hh, "%02d", flip_disp_hour(now->tm_hour, st->time_24h));
  snprintf(mm, sizeof mm, "%02d", now->tm_min);
  int s = st->steps; if (s < 0 || s > 99999) s = 0;
  snprintf(steps, sizeof steps, "%d", s);
  flip_format_date(date, sizeof date, now, st->date_fmt, st->lang);
  snprintf(batt, sizeof batt, "%d%%", st->battery);

  // Measured from the design DOM (×0.667): steps y=11, hero y=58 h=72,
  // date y=138, battery y=173 — pinned, not derived.
  const int heroH = 64;
  const int y_steps = top + 11;
  const int y_hero  = top + 58;
  const int y_date  = bot - 90;
  const int y_batt  = bot - 55;

  // STEPS (top) — ramp colour.
  row(ctx, flip_lbl_steps(st->lang), steps, flip_step_color(st->steps, st->step_goal), left, w, y_steps);

  // HERO clock — right-aligned per design (v01-hero justify-content:flex-end):
  // MM ends at the right padding edge, square colon dots sit left of it, HH
  // right-aligned ending before the colon. Doto digit advance @60px = 36px.
  (void)cx;
  const int sq = 7;
  const int right = left + w - PAD;
  const int mm_x = right - 2 * GCW;                  // left edge of MM block
  const int sqx = mm_x - 8 - sq;                     // colon square left edge
  const int hh_x = sqx - 8 - 2 * GCW;
  {
    time_t pt = time(NULL) - 60;
    struct tm *pv = localtime(&pt);
    char prev[8];
    snprintf(prev, sizeof prev, "%02d%02d", flip_disp_hour(pv->tm_hour, st->time_24h), pv->tm_min);
    char cur[8];
    snprintf(cur, sizeof cur, "%s%s", hh, mm);
    const int xs[4] = {hh_x, hh_x + GCW, mm_x, mm_x + GCW};
    for (int i = 0; i < 4; i++) {
      GRect r = GRect(xs[i], y_hero, GCW, GCH);
      if (st->anim > 0 && prev[i] != cur[i])
        flap_cell(ctx, s_dg[prev[i] - '0'], s_dg[cur[i] - '0'], r, st->anim, GColorBlack);
      else {
        graphics_context_set_compositing_mode(ctx, GCompOpAssign);
        graphics_draw_bitmap_in_rect(ctx, s_dg[cur[i] - '0'], r);
      }
    }
  }
  graphics_context_set_fill_color(ctx, COL_TEXT);
  const int midY = y_hero + heroH / 2;
  graphics_fill_rect(ctx, GRect(sqx, midY - 13, sq, sq), 1, GCornersAll);
  graphics_fill_rect(ctx, GRect(sqx, midY + 6,  sq, sq), 1, GCornersAll);

  // DATE + BATTERY.
  row(ctx, flip_lbl_date(st->lang), date, COL_TEXT, left, w, y_date);
  row(ctx, flip_lbl_batt(st->lang), batt, COL_TEXT, left, w, y_batt);
}

int main(void) {
  flip_run(render, COL_INK, setup);
}
