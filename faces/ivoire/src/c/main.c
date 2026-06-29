// FlipBoard · IVOIRE (face 03) — flip-clock per design V03 (sc-flip):
// DARK field (#14161b) with two IVORY clock tiles (Oswald dark digits),
// light values elsewhere. Steps top (label left, ramp value right),
// footer DATE (left) + BATTERIE (right, small % suffix). ×0.667 scale.

#include "flipboard.h"

#define PAD 12
#define COL_BG     GColorBlack                       // design #14161b → black in 64-col
#define COL_TILE   GColorPastelYellow                // ivory tile (#ece7da → closest 64-col ivory tint)
#define COL_SEAM   GColorLightGray                   // fold seam on tile
#define COL_DIGIT  GColorBlack                       // tile digits (#23211c)
#define COL_TXT    GColorWhite                       // light values (#e7e1d4)
#define COL_LABEL  GColorLightGray                   // labels (#80838d → keep neutral grey)

static GFont s_sub;    // Oswald 28 (design 42px × 0.667)

// Hero digits: pre-baked anti-aliased Oswald 52 sprites, dark ink on the ivory
// tile colour (tools/gen_glyphs.py) — sheet order "0123456789".
#define GCW 34
#define GCH 68
static GBitmap *s_osw, *s_dg[10];

static void setup(void) {
  s_sub = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_OSWALD_28));
  s_osw = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_OSW52);
  for (int i = 0; i < 10; i++)
    s_dg[i] = gbitmap_create_as_sub_bitmap(s_osw, GRect(i * GCW, 0, GCW, GCH));
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


// Ivory flip tile (rounded rect + seam) with two baked digit sprites centred.
// On minute change, digits that changed run the split-flap fold.
static void ctile(GContext *ctx, GRect r, const char *val, const char *prev, int anim) {
  graphics_context_set_fill_color(ctx, COL_TILE);
  graphics_fill_rect(ctx, r, 6, GCornersAll);
  int gx = r.origin.x + (r.size.w - 2 * GCW) / 2;
  int gy = r.origin.y + (r.size.h - GCH) / 2;
  for (int i = 0; i < 2; i++) {
    GRect cr = GRect(gx + i * GCW, gy, GCW, GCH);
    if (anim > 0 && prev[i] != val[i])
      flap_cell(ctx, s_dg[prev[i] - '0'], s_dg[val[i] - '0'], cr, anim, COL_TILE);
    else {
      graphics_context_set_compositing_mode(ctx, GCompOpAssign);
      graphics_draw_bitmap_in_rect(ctx, s_dg[val[i] - '0'], cr);
    }
  }
  int sy = r.origin.y + r.size.h / 2;
  graphics_context_set_stroke_color(ctx, COL_SEAM);
  graphics_draw_line(ctx, GPoint(r.origin.x + 2, sy), GPoint(r.origin.x + r.size.w - 2, sy));
}

static void render(GContext *ctx, const FlipState *st, struct tm *now, GRect area) {
  const int left = area.origin.x, top = area.origin.y;
  const int w = area.size.w, h = area.size.h, bot = top + h;

  graphics_context_set_fill_color(ctx, COL_BG);
  graphics_fill_rect(ctx, area, 0, GCornerNone);

  char hh[4], mm[4], steps[16], date[12], batt[8];
  snprintf(hh,    sizeof hh,    "%02d", flip_disp_hour(now->tm_hour, st->time_24h));
  snprintf(mm,    sizeof mm,    "%02d", now->tm_min);
  int s = st->steps; if (s < 0 || s > 99999) s = 0;
  snprintf(steps, sizeof steps, "%d", s);
  flip_format_date(date, sizeof date, now, st->date_fmt, st->lang);
  snprintf(batt,  sizeof batt,  "%d", st->battery);

  GFont g14b = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
  GFont g14  = fonts_get_system_font(FONT_KEY_GOTHIC_14);

  // ── TOP: PAS label (left, bottom-aligned with value) + ramp value (right) ──
  graphics_context_set_text_color(ctx, COL_LABEL);
  graphics_draw_text(ctx, flip_lbl_steps(st->lang), g14b,
                     GRect(left + PAD, top + 24, 70, 16),
                     GTextOverflowModeFill, GTextAlignmentLeft, NULL);
  graphics_context_set_text_color(ctx, flip_step_color(s, st->step_goal));
  graphics_draw_text(ctx, steps, s_sub,
                     GRect(left + PAD, top + 6, w - 2*PAD, 34),
                     GTextOverflowModeFill, GTextAlignmentRight, NULL);

  // ── HERO: two ivory clock tiles + light colon dots (design v03-hero) ──
  // Tiles enlarged for legibility (Rodolphe 2026-06-23): bigger baked digits
  // (cell 34×68) in 82×72 tiles, gap 16 → 180px wide, ~10px side margins (safe).
  const int tileH = 72, tileW = 82, cgap = 16;
  (void)h;
  const int tileY = top + 72;
  const int tile1x = left + w / 2 - cgap / 2 - tileW;
  const int tile2x = left + w / 2 + cgap / 2;
  const int colX = left + w / 2;
  const int colMid = tileY + tileH / 2;

  {
    time_t pt = time(NULL) - 60;
    struct tm *pv = localtime(&pt);
    char ph[4], pm[4];
    snprintf(ph, sizeof ph, "%02d", flip_disp_hour(pv->tm_hour, st->time_24h));
    snprintf(pm, sizeof pm, "%02d", pv->tm_min);
    ctile(ctx, GRect(tile1x, tileY, tileW, tileH), hh, ph, st->anim);
    ctile(ctx, GRect(tile2x, tileY, tileW, tileH), mm, pm, st->anim);
  }

  // Colon: two light round dots between the tiles (design v03-colon)
  graphics_context_set_fill_color(ctx, COL_TXT);
  graphics_fill_circle(ctx, GPoint(colX, colMid - 9), 3);
  graphics_fill_circle(ctx, GPoint(colX, colMid + 9), 3);

  // ── FOOTER: DATE (left cell) + BATTERIE (right cell, % suffix) ──
  graphics_context_set_text_color(ctx, COL_LABEL);
  graphics_draw_text(ctx, flip_lbl_date(st->lang), g14,
                     GRect(left + PAD, bot - 52, 80, 14),
                     GTextOverflowModeFill, GTextAlignmentLeft, NULL);
  graphics_draw_text(ctx, flip_lbl_batt(st->lang), g14,
                     GRect(left + w / 2, bot - 52, w / 2 - PAD, 14),
                     GTextOverflowModeFill, GTextAlignmentRight, NULL);
  graphics_context_set_text_color(ctx, COL_TXT);
  graphics_draw_text(ctx, date, s_sub,
                     GRect(left + PAD, bot - 40, 90, 34),
                     GTextOverflowModeFill, GTextAlignmentLeft, NULL);
  // battery: number right-aligned before a small Gothic "%" at the right edge
  graphics_draw_text(ctx, batt, s_sub,
                     GRect(left + w / 2, bot - 40, w / 2 - PAD - 14, 34),
                     GTextOverflowModeFill, GTextAlignmentRight, NULL);
  graphics_context_set_text_color(ctx, COL_LABEL);
  graphics_draw_text(ctx, "%", g14,
                     GRect(left + w - PAD - 12, bot - 28, 12, 14),
                     GTextOverflowModeFill, GTextAlignmentLeft, NULL);
}

int main(void) {
  flip_run(render, COL_BG, setup);
}
