// FlipBoard · QUAI (face 05) — amber 7-segment platform board.
// The digits are PRE-BAKED anti-aliased sprites (tools/gen_glyphs.py): ghost
// '8' + live glyph composited in design amber (#f6a91e) and quantized to the
// 64-colour palette by DISPLAYED value (e-paper LUT). This sidesteps Pebble's
// 1-bit font rasteriser entirely — the watch shows the design's LCD look.
// Layout measured from the design DOM (×0.667). Labels use Gothic 14.

#include "flipboard.h"

#define PAD 10
#define COL_BG      GColorBlack
#define COL_LBL     GColorFromRGB(170, 170, 0)  // muted amber labels #a98a3e (display-calibrated)
#define COL_SEP     GColorFromRGB(85, 85, 0)    // hairline above footer

// Sprite sheets: glyph order "0123456789:-", one cell per glyph.
#define CW46 35
#define CH46 50
#define CW24 22
#define CH24 27

static GBitmap *s_sheet46, *s_sheet24;
static GBitmap *s_g46[12], *s_g24[12];

static int gidx(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c == ':') return 10;
  return 11;  // '-'
}

static void setup(void) {
  s_sheet46 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_GLYPHS46);
  s_sheet24 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_GLYPHS24);
  for (int i = 0; i < 12; i++) {
    s_g46[i] = gbitmap_create_as_sub_bitmap(s_sheet46, GRect(i * CW46, 0, CW46, CH46));
    s_g24[i] = gbitmap_create_as_sub_bitmap(s_sheet24, GRect(i * CW24, 0, CW24, CH24));
  }
}


// Split-flap fold illusion for one sprite cell: the old glyph's top half
// folds away, then the new glyph unrolls from the seam. f = st->anim (5..0).
static void flap_cell(GContext *ctx, GBitmap *oldb, GBitmap *newb, GRect r, int f) {
  const int hh = r.size.h / 2;
  graphics_context_set_compositing_mode(ctx, GCompOpAssign);
  if (f >= 5) {
    graphics_draw_bitmap_in_rect(ctx, oldb, r);
  } else if (f == 4 || f == 3) {
    graphics_draw_bitmap_in_rect(ctx, oldb, r);
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, GRect(r.origin.x, r.origin.y, r.size.w, f == 4 ? hh / 2 : hh), 0, GCornerNone);
  } else {
    graphics_draw_bitmap_in_rect(ctx, newb, r);
    if (f > 0) {
      graphics_context_set_fill_color(ctx, GColorBlack);
      int bh = (f == 2) ? hh : hh / 2;
      graphics_fill_rect(ctx, GRect(r.origin.x, r.origin.y + hh, r.size.w, bh), 0, GCornerNone);
    }
  }
}

// Blit a string of baked glyph cells starting at x (or ending at x when
// right_align). Cells are opaque — the background is baked in.
static void blit(GContext *ctx, GBitmap **set, int cw, int ch,
                 const char *s, int x, int y, bool right_align) {
  int n = 0;
  for (const char *p = s; *p; p++) n++;
  if (right_align) x -= n * cw;
  graphics_context_set_compositing_mode(ctx, GCompOpAssign);
  for (const char *p = s; *p; p++, x += cw)
    graphics_draw_bitmap_in_rect(ctx, set[gidx(*p)], GRect(x, y, cw, ch));
}

static void render(GContext *ctx, const FlipState *st, struct tm *now, GRect area) {
  const int left = area.origin.x, top = area.origin.y;
  const int w = area.size.w, h = area.size.h, bot = top + h;
  (void)h;

  graphics_context_set_fill_color(ctx, COL_BG);
  graphics_fill_rect(ctx, area, 0, GCornerNone);

  char hhmm[8], steps[16], date[12], batt[8];
  snprintf(hhmm,  sizeof hhmm,  "%02d:%02d", flip_disp_hour(now->tm_hour, st->time_24h), now->tm_min);
  int s = st->steps; if (s < 0 || s > 99999) s = 0;
  snprintf(steps, sizeof steps, "%d", s);
  flip_format_date(date, sizeof date, now, st->date_fmt, st->lang, st->year_fmt);
  snprintf(batt,  sizeof batt,  "%d", st->battery);  // '%' drawn as Gothic suffix

  GFont g14  = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  GFont g14b = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);

  // Header: "> DEPARTS" (muted amber) + weekday (right, dimmer)
  graphics_context_set_text_color(ctx, COL_LBL);
  graphics_draw_text(ctx, st->lang == FLIP_FR ? "> DEPARTS" : "> DEPARTURES", g14b,
                     GRect(left + PAD, top + 4, w - 2*PAD, 18),
                     GTextOverflowModeFill, GTextAlignmentLeft, NULL);
  graphics_draw_text(ctx, flip_weekday(st->lang, now->tm_wday), g14b,
                     GRect(left + PAD, top + 4, w - 2*PAD, 18),
                     GTextOverflowModeFill, GTextAlignmentRight, NULL);

  // HERO clock: five 35px baked cells centred, y=54 (measured from design).
  // On minute change, cells whose digit changed run the split-flap fold.
  {
    time_t pt = time(NULL) - 60;
    struct tm *pv = localtime(&pt);
    char prev[8];
    snprintf(prev, sizeof prev, "%02d:%02d", flip_disp_hour(pv->tm_hour, st->time_24h), pv->tm_min);
    int hx = left + (w - 5 * CW46) / 2;
    for (int i = 0; i < 5; i++) {
      GRect r = GRect(hx + i * CW46, top + 54, CW46, CH46);
      if (st->anim > 0 && prev[i] != hhmm[i])
        flap_cell(ctx, s_g46[gidx(prev[i])], s_g46[gidx(hhmm[i])], r, st->anim);
      else {
        graphics_context_set_compositing_mode(ctx, GCompOpAssign);
        graphics_draw_bitmap_in_rect(ctx, s_g46[gidx(hhmm[i])], r);
      }
    }
  }

  // STEPS row: label left, baked amber digits right (y measured)
  graphics_context_set_text_color(ctx, COL_LBL);
  graphics_draw_text(ctx, flip_lbl_steps(st->lang), g14b,
                     GRect(left + PAD, bot - 85, w / 2 - PAD, 16),
                     GTextOverflowModeFill, GTextAlignmentLeft, NULL);
  blit(ctx, s_g24, CW24, CH24, steps, left + w - PAD, bot - 89, true);

  // Hairline separator above footer
  graphics_context_set_fill_color(ctx, COL_SEP);
  graphics_fill_rect(ctx, GRect(left + PAD, bot - 60, w - 2*PAD, 1), 0, GCornerNone);

  // Footer: DATE (left) + BATTERIE (right, Gothic % suffix)
  graphics_context_set_text_color(ctx, COL_LBL);
  graphics_draw_text(ctx, flip_lbl_date(st->lang), g14,
                     GRect(left + PAD, bot - 58, w / 2 - PAD, 14),
                     GTextOverflowModeFill, GTextAlignmentLeft, NULL);
  graphics_draw_text(ctx, flip_lbl_batt(st->lang), g14,
                     GRect(left + w / 2, bot - 58, w / 2 - PAD, 14),
                     GTextOverflowModeFill, GTextAlignmentLeft, NULL);
  blit(ctx, s_g24, CW24, CH24, date, left + PAD, bot - 40, false);
  blit(ctx, s_g24, CW24, CH24, batt, left + w - PAD - 14, bot - 40, true);
  graphics_context_set_text_color(ctx, COL_LBL);
  graphics_draw_text(ctx, "%", g14,
                     GRect(left + w - PAD - 12, bot - 28, 12, 14),
                     GTextOverflowModeFill, GTextAlignmentLeft, NULL);
}

int main(void) {
  flip_run(render, COL_BG, setup);
}
