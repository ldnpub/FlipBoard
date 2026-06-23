// FlipBoard · SOLARI (face 02) — mechanical split-flap board (Silkscreen).
// Layout per design V02 (mockup 300px → Pebble 200px, ×0.667): each character
// sits on its OWN flap tile (design .flapwrap.tile) with a fold seam. Fonts at
// design-true sizes: hero 40px, values 24px (Silkscreen pixel grid = ×8).

#include "flipboard.h"

#define PAD 12
#define COL_BG    GColorBlack
#define COL_FLAP  GColorDarkGray   // tile face (design #101216 → must stay visible on black)
#define COL_SEAM  GColorBlack      // fold line across each tile
#define COL_TXT   GColorWhite
#define COL_LBL   GColorLightGray  // label grey #7e8087 — LightGray reads closest on panel
#define COL_DOT   GColorChromeYellow

static GFont s_hero;   // Silkscreen 44 (design 66px × 0.667)
static GFont s_val;    // Silkscreen 24

static void setup(void) {
  s_hero = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_SILK_44));
  s_val  = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_SILK_24));
}

// Per-character flap tiles. dw/cw = digit/colon advance for the font size,
// th = tile height, voff = text y offset inside tile. x anchored by `endx`
// (right edge) when center==false, else centred on `cx`.
// Advance per char, scaled from the digit width (Silkscreen @1em: digit .75,
// colon .375, slash .625, percent .875 — measured from the TTF).
static int adv(char c, int dw) {
  if (c == ':') return dw / 2;
  if (c == '/') return (dw * 5) / 6;
  if (c == '%') return (dw * 7) / 6;
  return dw;
}
static int tiles_width(const char *s, int dw) {
  int tw = 0;
  for (const char *p = s; *p; p++) tw += adv(*p, dw) + 3;  // +2 pad +1 gap
  return tw - 1;                                           // no trailing gap
}
static void tilerow(GContext *ctx, const char *s, GFont f, int dw,
                    int th, int voff, int anchor_x, bool center, int y, GColor tc) {
  int total = tiles_width(s, dw);
  int x = center ? anchor_x - total / 2 : anchor_x - total;
  char one[2] = {0, 0};
  for (const char *p = s; *p; p++) {
    int gw = adv(*p, dw) + 2;
    // Design tiles are near-black, defined by their edges — black fill with a
    // subtle grey outline reads far closer than a solid grey block.
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, GRect(x, y, gw, th), 2, GCornersAll);
    graphics_context_set_stroke_color(ctx, COL_FLAP);
    graphics_draw_round_rect(ctx, GRect(x, y, gw, th), 2);
    graphics_draw_line(ctx, GPoint(x + 1, y + th / 2), GPoint(x + gw - 2, y + th / 2));
    one[0] = *p;
    graphics_context_set_text_color(ctx, tc);
    graphics_draw_text(ctx, one, f, GRect(x, y + voff, gw, th),
                       GTextOverflowModeFill, GTextAlignmentCenter, NULL);
    x += gw + 1;
  }
}

// Advances measured from Silkscreen.ttf: digit = 0.75em, colon = 0.375em.
#define DW44 33
#define DW24 18
#define TH44 49
#define TH24 25

static void render(GContext *ctx, const FlipState *st, struct tm *now, GRect area) {
  const int left = area.origin.x, top = area.origin.y;
  const int w = area.size.w, h = area.size.h, bot = top + h;
  (void)h;

  graphics_context_set_fill_color(ctx, COL_BG);
  graphics_fill_rect(ctx, area, 0, GCornerNone);

  char hhmm[8], steps[16], date[24], batt[16];
  snprintf(hhmm, sizeof hhmm, "%02d:%02d", now->tm_hour, now->tm_min);
  int s = st->steps; if (s < 0 || s > 99999) s = 0;
  snprintf(steps, sizeof steps, "%05d", s);   // 5-digit board padding
  snprintf(date, sizeof date, "%02d/%02d", now->tm_mday, now->tm_mon + 1);
  snprintf(batt, sizeof batt, "%d%%", st->battery);

  GFont g14  = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  GFont g14b = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);

  // Header: DEPARTS + amber dot (design v02-head).
  graphics_context_set_text_color(ctx, COL_TXT);
  graphics_draw_text(ctx, st->lang == FLIP_FR ? "DEPARTS" : "DEPARTURES", g14b,
                     GRect(left + PAD, top + 2, w - 2*PAD, 18),
                     GTextOverflowModeFill, GTextAlignmentLeft, NULL);
  graphics_context_set_fill_color(ctx, COL_DOT);
  graphics_fill_circle(ctx, GPoint(left + w - PAD - 4, top + 11), 4);

  // Row anchors — 4 rows spread between header and bottom (design space-around).
  const int right = left + w - PAD;
  // Measured from the design DOM (×0.667): steps y=37 h=25, time y=77 h=49,
  // date y=140, battery y=178.
  const int y_steps = top + 37;
  const int y_time  = top + 77;
  const int y_date  = bot - 88;
  const int y_batt  = bot - 50;

  // STEPS: label left, 5 digit tiles right (ramp colour).
  graphics_context_set_text_color(ctx, COL_LBL);
  graphics_draw_text(ctx, flip_lbl_steps(st->lang), g14,
                     GRect(left + PAD, y_steps + 8, 70, 14),
                     GTextOverflowModeFill, GTextAlignmentLeft, NULL);
  tilerow(ctx, steps, s_val, DW24, TH24, -2, right, false, y_steps,
          flip_step_color(st->steps, st->step_goal));

  // HERO time: per-char tiles, centred.
  tilerow(ctx, hhmm, s_hero, DW44, TH44, -3, left + w / 2, true, y_time, COL_TXT);

  // DATE + BATTERY rows.
  graphics_context_set_text_color(ctx, COL_LBL);
  graphics_draw_text(ctx, flip_lbl_date(st->lang), g14,
                     GRect(left + PAD, y_date + 8, 70, 14),
                     GTextOverflowModeFill, GTextAlignmentLeft, NULL);
  tilerow(ctx, date, s_val, DW24, TH24, -2, right, false, y_date, COL_TXT);

  graphics_context_set_text_color(ctx, COL_LBL);
  graphics_draw_text(ctx, flip_lbl_batt(st->lang), g14,
                     GRect(left + PAD, y_batt + 8, 70, 14),
                     GTextOverflowModeFill, GTextAlignmentLeft, NULL);
  tilerow(ctx, batt, s_val, DW24, TH24, -2, right, false, y_batt, COL_TXT);
}

int main(void) {
  flip_run(render, COL_BG, setup);
}
