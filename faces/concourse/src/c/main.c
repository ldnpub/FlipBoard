// FlipBoard · CONCOURSE (face 06) — bilingual 14-segment departure board.
// Values are PRE-BAKED anti-aliased DSEG14 sprites (tools/gen_glyphs.py):
// white with grey AA edges for the standard rows, and five ramp-tinted 25px
// sheets for the steps row (red near 0 → green toward the 10000 goal).
// This sidesteps Pebble's 1-bit font rasteriser. Layout per design V06.

#include "flipboard.h"

#define PAD 10
#define COL_BG     GColorFromRGB(0x0A, 0x0C, 0x0E)
#define COL_BAR    GColorDarkGray
#define COL_TXT    GColorWhite
#define COL_LBL    GColorFromRGB(0x8A, 0x8C, 0x93)
#define COL_GREEN  GColorFromRGB(0, 170, 170)  // design green #5ec98a — displays (87,165,162), closest on panel
#define COL_RED    GColorRed
#define COL_DOT    GColorFromRGB(0, 170, 170)  // green dot, display-calibrated

// Sprite sheets. White sheet order "0123456789:-"; steps sheets "0123456789".
#define WCW 19
#define WCH 25
#define SCW 21
#define SCH 28

static GBitmap *s_white, *s_wg[12];
static GBitmap *s_steps_sheet[5];
static GBitmap *s_sg[5][10];

static int gidx(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c == ':') return 10;
  return 11;  // '-'
}
static int ramp_band(int s, int goal) {
  if (goal < 1) goal = FLIP_STEP_GOAL_DEFAULT;
  int pct = (s <= 0) ? 0 : (s >= goal ? 100 : (s * 100) / goal);
  if (pct < 20) return 0;
  if (pct < 40) return 1;
  if (pct < 65) return 2;
  if (pct < 90) return 3;
  return 4;
}

static void setup(void) {
  s_white = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SEG22);
  for (int i = 0; i < 12; i++)
    s_wg[i] = gbitmap_create_as_sub_bitmap(s_white, GRect(i * WCW, 0, WCW, WCH));
  const uint32_t res[5] = {
    RESOURCE_ID_IMAGE_STEPS_RED, RESOURCE_ID_IMAGE_STEPS_ORANGE,
    RESOURCE_ID_IMAGE_STEPS_YELLOW, RESOURCE_ID_IMAGE_STEPS_LGREEN,
    RESOURCE_ID_IMAGE_STEPS_GREEN,
  };
  for (int b = 0; b < 5; b++) {
    s_steps_sheet[b] = gbitmap_create_with_resource(res[b]);
    for (int i = 0; i < 10; i++)
      s_sg[b][i] = gbitmap_create_as_sub_bitmap(s_steps_sheet[b], GRect(i * SCW, 0, SCW, SCH));
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

static void blit(GContext *ctx, GBitmap **set, int cw, int ch,
                 const char *s, int x, int y) {
  graphics_context_set_compositing_mode(ctx, GCompOpAssign);
  for (const char *p = s; *p; p++, x += cw)
    graphics_draw_bitmap_in_rect(ctx, set[gidx(*p)], GRect(x, y, cw, ch));
}

// One board row: stacked FR+EN label (left) | sprite value | status (right).
static void brow(GContext *ctx, int x, int w, int y,
                 const char *fr, const char *en,
                 const char *val, GBitmap **set, int cw, int ch,
                 const char *status, GColor sc) {
  const int ip = 6, lbl_w = 40, stat_w = 52;
  graphics_context_set_text_color(ctx, GColorLightGray);
  graphics_draw_text(ctx, fr, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD),
                     GRect(x + ip, y + 4, lbl_w, 14), GTextOverflowModeFill, GTextAlignmentLeft, NULL);
  graphics_context_set_text_color(ctx, COL_LBL);
  graphics_draw_text(ctx, en, fonts_get_system_font(FONT_KEY_GOTHIC_14),
                     GRect(x + ip, y + 19, lbl_w, 14), GTextOverflowModeFill, GTextAlignmentLeft, NULL);
  if (val) blit(ctx, set, cw, ch, val, x + ip + lbl_w, y + 4);
  if (status) {
    graphics_context_set_text_color(ctx, sc);
    graphics_draw_text(ctx, status, fonts_get_system_font(FONT_KEY_GOTHIC_14),
                       GRect(x + w - ip - stat_w, y + 4, stat_w, 14),
                       GTextOverflowModeFill, GTextAlignmentRight, NULL);
  }
}

static void render(GContext *ctx, const FlipState *st, struct tm *now, GRect area) {
  const int left = area.origin.x, top = area.origin.y;
  const int w = area.size.w, h = area.size.h;

  graphics_context_set_fill_color(ctx, COL_BG);
  graphics_fill_rect(ctx, area, 0, GCornerNone);

  char hhmm[8], steps[16], date[24], batt[16], pct[8];
  snprintf(hhmm, sizeof hhmm, "%02d:%02d", now->tm_hour, now->tm_min);
  int s = st->steps; if (s < 0 || s > 99999) s = 0;
  snprintf(steps, sizeof steps, "%05d", s);
  snprintf(date, sizeof date, "%02d-%02d", now->tm_mday, now->tm_mon + 1);
  snprintf(batt, sizeof batt, "%d", st->battery);
  snprintf(pct, sizeof pct, "%d%%", flip_progress_pct(s, st->step_goal));

  // Header bar — design #101317 is near-black, defined by its bottom hairline.
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(left, top, w, 24), 0, GCornerNone);
  graphics_context_set_fill_color(ctx, COL_BAR);
  graphics_fill_rect(ctx, GRect(left, top + 23, w, 1), 0, GCornerNone);
  graphics_context_set_text_color(ctx, COL_TXT);
  graphics_draw_text(ctx, "PEBBLE-DEPARTS", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD),
                     GRect(left + PAD, top + 3, 108, 18),
                     GTextOverflowModeFill, GTextAlignmentLeft, NULL);
  graphics_draw_text(ctx, hhmm, fonts_get_system_font(FONT_KEY_GOTHIC_14),
                     GRect(left + PAD, top + 3, w - 2*PAD - 12, 18),
                     GTextOverflowModeFill, GTextAlignmentRight, NULL);
  graphics_context_set_fill_color(ctx, COL_DOT);
  graphics_fill_circle(ctx, GPoint(left + w - PAD - 3, top + 12), 3);

  // Four rows filling the space under the bar.
  const int y0 = top + 30, rh = (h - 34) / 4;
  brow(ctx, left, w, y0 + 0*rh, "PAS", "STEPS", steps,
       s_sg[ramp_band(s, st->step_goal)], SCW, SCH, pct, flip_step_color(s, st->step_goal));
  brow(ctx, left, w, y0 + 1*rh, "HEURE", "TIME", NULL,
       s_wg, WCW, WCH, st->lang == FLIP_FR ? "A L HEURE" : "ON TIME", COL_GREEN);
  {
    // HEURE value drawn cell by cell so changed digits run the split-flap fold
    time_t pt = time(NULL) - 60;
    struct tm *pv = localtime(&pt);
    char prev[8];
    snprintf(prev, sizeof prev, "%02d:%02d", pv->tm_hour, pv->tm_min);
    int vx = left + 6 + 40, vy = y0 + 1*rh + 4;
    for (int i = 0; i < 5; i++) {
      GRect r = GRect(vx + i * WCW, vy, WCW, WCH);
      if (st->anim > 0 && prev[i] != hhmm[i])
        flap_cell(ctx, s_wg[gidx(prev[i])], s_wg[gidx(hhmm[i])], r, st->anim);
      else {
        graphics_context_set_compositing_mode(ctx, GCompOpAssign);
        graphics_draw_bitmap_in_rect(ctx, s_wg[gidx(hhmm[i])], r);
      }
    }
  }
  brow(ctx, left, w, y0 + 2*rh, "DATE", "DATE", date,
       s_wg, WCW, WCH, flip_weekday(st->lang, now->tm_wday), COL_LBL);
  brow(ctx, left, w, y0 + 3*rh, "BATT", "BATT", batt,
       s_wg, WCW, WCH, st->battery < 20 ? (st->lang == FLIP_FR ? "FAIBLE" : "LOW") : "OK",
       st->battery < 20 ? COL_RED : COL_GREEN);
  // small "%" suffix after the battery digits (design br-suf)
  {
    int ndig = st->battery >= 100 ? 3 : (st->battery >= 10 ? 2 : 1);
    int bx = left + 6 + 40 + ndig * WCW + 4;
    graphics_context_set_text_color(ctx, COL_LBL);
    graphics_draw_text(ctx, "%", fonts_get_system_font(FONT_KEY_GOTHIC_14),
                       GRect(bx, y0 + 3*rh + 12, 14, 14),
                       GTextOverflowModeFill, GTextAlignmentLeft, NULL);
  }
}

int main(void) {
  flip_run(render, COL_BG, setup);
}
