// FlipBoard · APOLLO (face 04) — aerospace console terminal (VT323).
// Layout per design V04 (mockup 300px → Pebble 200px, scale ×0.667):
//   status "> SYS-OK | 24H" → PAS / 10000 → big steps (left) → progress bar
//   → HERO clock (VT323 68px, centred) → footer DATE | BATTERIE.
// Labels use Gothic 14 (design 12px mono equivalent — crisper than tiny VT323).

#include "flipboard.h"

#define PAD 10
#define COL_BG     GColorFromRGB(0x0C, 0x0D, 0x0F)
#define COL_TXT    GColorWhite
#define COL_GREEN  GColorGreen
#define COL_DIM    GColorFromRGB(85, 170, 85)     // grey-green #8a9084 — displays (117,157,118) on panel
#define COL_TRACK  GColorDarkGray                   // progress bar track (lowest visible grey on black)

static GFont s_steps;  // VT323 36 — big step count
static GFont s_txt;    // VT323 28 — footer values

// Hero digits are pre-baked anti-aliased sprites (tools/gen_glyphs.py):
// VT323 68px white with grey AA edges — sheet order "0123456789:".
#define HCW 28
#define HCH 72
static GBitmap *s_vt68, *s_hg[11];

static void setup(void) {
  s_steps = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_VT_36));
  s_txt   = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_VT_28));
  s_vt68  = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_VT68);
  for (int i = 0; i < 11; i++)
    s_hg[i] = gbitmap_create_as_sub_bitmap(s_vt68, GRect(i * HCW, 0, HCW, HCH));
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

// VT323 is too thin for the 1-bit Pebble rasteriser at large sizes — strokes
// come out ragged with holes. Drawing 4× with 1px offsets synthesises the
// chunky terminal weight the design shows (web rendering is anti-aliased).
static void bold_text(GContext *ctx, const char *s, GFont f, GRect r,
                      GTextAlignment align, GColor c) {
  graphics_context_set_text_color(ctx, c);
  for (int dx = 0; dx <= 1; dx++)
    for (int dy = 0; dy <= 1; dy++)
      graphics_draw_text(ctx, s, f, GRect(r.origin.x + dx, r.origin.y + dy, r.size.w, r.size.h),
                         GTextOverflowModeFill, align, NULL);
}

static void render(GContext *ctx, const FlipState *st, struct tm *now, GRect area) {
  const int left = area.origin.x, top = area.origin.y;
  const int w = area.size.w, h = area.size.h, bot = top + h;

  graphics_context_set_fill_color(ctx, COL_BG);
  graphics_fill_rect(ctx, area, 0, GCornerNone);

  // Clamp steps — health emulator can return 1e9 as a sentinel value
  int steps = st->steps;
  if (steps < 0 || steps > 99999) steps = 0;

  char hhmm[8], stepstr[8], date[12], batt[8];
  snprintf(hhmm,    sizeof hhmm,    "%02d:%02d", now->tm_hour, now->tm_min);
  snprintf(stepstr, sizeof stepstr, "%05d", steps);
  snprintf(date,    sizeof date,    "%02d/%02d", now->tm_mday, now->tm_mon + 1);
  snprintf(batt,    sizeof batt,    "%d%%", st->battery);

  GFont g14  = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  GFont g14b = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);

  // Status line: "> SYS-OK" (green) + "24H" (dim) — design v04-status
  graphics_context_set_text_color(ctx, COL_GREEN);
  graphics_draw_text(ctx, "> SYS-OK", g14b, GRect(left + PAD, top + 2, w - 2*PAD, 16),
                     GTextOverflowModeFill, GTextAlignmentLeft, NULL);
  graphics_context_set_text_color(ctx, COL_DIM);
  graphics_draw_text(ctx, "24H", g14, GRect(left + PAD, top + 2, 122, 16),
                     GTextOverflowModeFill, GTextAlignmentRight, NULL);
  // blinking cursor block at the right edge (design v04-cur)
  graphics_context_set_fill_color(ctx, COL_GREEN);
  graphics_fill_rect(ctx, GRect(left + w - PAD - 6, top + 5, 6, 12), 0, GCornerNone);
  // hairline under status (design border-bottom)
  graphics_context_set_fill_color(ctx, COL_TRACK);
  graphics_fill_rect(ctx, GRect(left + PAD, top + 20, w - 2*PAD, 1), 0, GCornerNone);

  // Steps block: "PAS" left + "/ 10000" right, then BIG count LEFT-aligned (design v04-steps)
  graphics_context_set_text_color(ctx, COL_DIM);
  graphics_draw_text(ctx, flip_lbl_steps(st->lang), g14,
                     GRect(left + PAD, top + 35, w - 2*PAD, 16),
                     GTextOverflowModeFill, GTextAlignmentLeft, NULL);
  char goal_rhs[16];
  snprintf(goal_rhs, sizeof goal_rhs, "/ %d", st->step_goal);
  graphics_draw_text(ctx, goal_rhs, g14,
                     GRect(left + PAD, top + 35, w - 2*PAD, 16),
                     GTextOverflowModeFill, GTextAlignmentRight, NULL);
  bold_text(ctx, stepstr, s_steps, GRect(left + PAD, top + 49, w - 2*PAD, 40),
            GTextAlignmentLeft, flip_step_color(steps, st->step_goal));

  // Progress bar
  GRect track = GRect(left + PAD, top + 99, w - 2*PAD, 5);
  graphics_context_set_fill_color(ctx, COL_TRACK);
  graphics_fill_rect(ctx, track, 2, GCornersAll);
  int fill_w = (track.size.w * flip_progress_pct(steps, st->step_goal)) / 100;
  if (fill_w > 0) {
    graphics_context_set_fill_color(ctx, flip_step_color(steps, st->step_goal));
    graphics_fill_rect(ctx, GRect(track.origin.x, track.origin.y, fill_w, 5), 2, GCornersAll);
  }

  // HERO clock — VT323 68px centred between bar and footer (design v04-hero flex:1)
  // Measured from the design DOM (×0.667): hero block y=103..184, glyphs y=103.
  const int foot_top = bot - 48;
  const int heroH = 72;
  const int heroY = top + 104;
  {
    // five baked cells centred; changed digits run the split-flap fold
    time_t pt = time(NULL) - 60;
    struct tm *pv = localtime(&pt);
    char prev[8];
    snprintf(prev, sizeof prev, "%02d:%02d", pv->tm_hour, pv->tm_min);
    int hx = left + (w - 5 * HCW) / 2;
    graphics_context_set_compositing_mode(ctx, GCompOpAssign);
    for (int i = 0; i < 5; i++) {
      int gi = (hhmm[i] == ':') ? 10 : hhmm[i] - '0';
      int pi = (prev[i] == ':') ? 10 : prev[i] - '0';
      GRect r = GRect(hx + i * HCW, heroY, HCW, HCH);
      if (st->anim > 0 && pi != gi)
        flap_cell(ctx, s_hg[pi], s_hg[gi], r, st->anim);
      else
        graphics_draw_bitmap_in_rect(ctx, s_hg[gi], r);
    }
  }

  // Footer: hairline, DATE (left) + BATTERIE (right) labels above VT323 values
  graphics_context_set_fill_color(ctx, COL_TRACK);
  graphics_fill_rect(ctx, GRect(left + PAD, foot_top - 2, w - 2*PAD, 1), 0, GCornerNone);
  graphics_context_set_text_color(ctx, COL_DIM);
  graphics_draw_text(ctx, flip_lbl_date(st->lang), g14,
                     GRect(left + PAD, foot_top, w - 2*PAD, 14),
                     GTextOverflowModeFill, GTextAlignmentLeft, NULL);
  graphics_draw_text(ctx, flip_lbl_batt(st->lang), g14,
                     GRect(left + PAD, foot_top, w - 2*PAD, 14),
                     GTextOverflowModeFill, GTextAlignmentRight, NULL);
  bold_text(ctx, date, s_txt, GRect(left + PAD, foot_top + 12, w - 2*PAD, 30),
            GTextAlignmentLeft, COL_TXT);
  bold_text(ctx, batt, s_txt, GRect(left + PAD, foot_top + 12, w - 2*PAD, 30),
            GTextAlignmentRight, COL_TXT);
}

int main(void) {
  flip_run(render, COL_BG, setup);
}
