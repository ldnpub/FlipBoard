// FlipBoard LUMEN — Pebble Time 2 (emery) LED dot-matrix departure board.
// Face 07: every glyph is a 5x7 cell; lit dots glow, unlit dots form a ghost
// grid that sells the LED look. Font-free for the values (hand-drawn dots);
// labels use the system Gothic font.
//
// Phone settings (Clay): language (9), daily step goal, and grid contrast.
//   - 24h HH:MM, steady colon (NEVER blinks), minute-resolution redraws only.
//   - Steps drive a red -> amber -> green ramp toward the (configurable) goal.
//   - Grid contrast: FULL / FAINT / OFF, set from the phone or kept default.
//   - Language also cycles on a wrist shake; all settings persist.

#include <pebble.h>

#define STEP_GOAL_DEFAULT 10000
#define SCREEN_W    200
#define SCREEN_H    228
#define PAD         12

#define DM_COLS     5
#define DM_ROWS     7
#define DM_ADV      (DM_COLS + 1)   // 1-cell gap between glyphs

#define PERSIST_LANG 1
#define PERSIST_GOAL 2
#define PERSIST_GRID 3
#define PERSIST_DATEFMT 4
#define PERSIST_SHAKE   5
#define PERSIST_T24     6
#define PERSIST_YEAR    7

// Latin-script languages (system font renders the labels, so no dot-matrix
// glyphs are needed). Keep FR=0 / EN=1 stable for back-compat with persist.
enum { LANG_FR = 0, LANG_EN = 1, LANG_DE = 2, LANG_ES = 3, LANG_IT = 4,
       LANG_NL = 5, LANG_PT = 6, LANG_PL = 7, LANG_SV = 8 };
#define LANG_COUNT 9
#define LIDX(l) (((l) >= 0 && (l) < LANG_COUNT) ? (l) : 0)

// Ghost-grid intensity is a phone-selectable percentage (0 = no grid / max
// contrast, 100 = full grid). Rendered via ordered (Bayer) density dithering
// so the slider stays smooth even at small pitches where dot radius is coarse.
#define GRID_DEFAULT 45

// How long the transient shake overlay (steps-left / destination) stays up.
#define OVERLAY_MS 3500

// messageKeys (declared in package.json) are emitted as runtime uint32_t
// symbols; the auto header doesn't extern them, so declare them here.
extern uint32_t MESSAGE_KEY_LANG;
extern uint32_t MESSAGE_KEY_STEP_GOAL;
extern uint32_t MESSAGE_KEY_GRID;
extern uint32_t MESSAGE_KEY_DATE_FMT;
extern uint32_t MESSAGE_KEY_SHAKE_ACT;
extern uint32_t MESSAGE_KEY_YEAR_FMT;

static Window *s_window;
static Layer  *s_layer;

static int s_steps   = 0;
static int s_battery = 0;
static int s_lang    = LANG_FR;
static int s_goal    = STEP_GOAL_DEFAULT;
static int s_grid    = GRID_DEFAULT;   // ghost-grid intensity %, phone-tunable
static int s_datefmt = 0;              // 0=AUTO,1=DD/MM,2=MM/DD,3=DD.MM,4=ISO
static int s_yearfmt = 0;              // 0=off,1=2-digit,2=4-digit
static int s_shake   = 0;              // 0=Off,1=Flip,2=Light,3=Steps,4=Dest,5=T24
static bool s_time24h;                 // 12/24h; default = system, toggled by shake

// Transient shake overlay (steps-left / random destination). Drawn full-bleed in
// SYSTEM fonts (not dm_draw) so it isn't limited to DM_FONT's numeric glyph set.
static int    s_overlay = 0;
static char   s_ov_main[16];
static char   s_ov_sub[16];
static GColor s_ov_col;

// Departure-board easter egg: a shake can flash a random destination code.
static const char *DEST[] = {
  "CDG","JFK","LHR","HND","SFO","DXB","SIN","AMS","FRA","NRT",
  "LAX","HKG","GVA","BCN","FCO","SYD","YUL","GRU","ICN","MAD" };

// ─────────────────────── date format + 12/24h helpers ──────────────────────
// date format select/persist values: 0=AUTO(by language),1=DD/MM,2=MM/DD,3=DD.MM,4=ISO
static int lumen_lang_datefmt(int lang){
  switch(lang){ case 1: return 2;            /* EN -> MM/DD */
                case 2: case 5: case 7: return 3; /* DE,NL,PL -> DD.MM */
                case 8: return 4;            /* SV -> ISO */
                default: return 1; }         /* FR,ES,IT,PT -> DD/MM */
}
// year_fmt: 0=off, 1=2-digit, 2=4-digit. Appended per format convention.
static void lumen_format_date(char*buf,size_t n,struct tm*now,int fmt,int lang,int year_fmt){
  if(fmt==0) fmt=lumen_lang_datefmt(lang);
  int d=now->tm_mday,m=now->tm_mon+1,y=now->tm_year+1900,yy=y%100;
  switch(fmt){
    case 2: // MDY
      if(year_fmt==1)      snprintf(buf,n,"%02d/%02d/%02d",m,d,yy);
      else if(year_fmt==2) snprintf(buf,n,"%02d/%02d/%04d",m,d,y);
      else                 snprintf(buf,n,"%02d/%02d",m,d);
      break;
    case 3: // DOT (DMY with '.')
      if(year_fmt==1)      snprintf(buf,n,"%02d.%02d.%02d",d,m,yy);
      else if(year_fmt==2) snprintf(buf,n,"%02d.%02d.%04d",d,m,y);
      else                 snprintf(buf,n,"%02d.%02d",d,m);
      break;
    case 4: // ISO
      if(year_fmt==1)      snprintf(buf,n,"%02d-%02d-%02d",yy,m,d);
      else                 snprintf(buf,n,"%04d-%02d-%02d",y,m,d);
      break;
    default: // DMY
      if(year_fmt==1)      snprintf(buf,n,"%02d/%02d/%02d",d,m,yy);
      else if(year_fmt==2) snprintf(buf,n,"%02d/%02d/%04d",d,m,y);
      else                 snprintf(buf,n,"%02d/%02d",d,m);
      break;
  }
}
// Hour to display, honouring the 12/24h setting (no AM/PM — space is tight).
static int lumen_disp_hour(int h24,bool h24flag){ if(h24flag) return h24; int h=h24%12; return h?h:12; }

// ─────────────────────────── 5x7 dot-matrix font ───────────────────────────
// Ported verbatim from design_handoff/dotmatrix.jsx DM_FONT. Each glyph is 7
// rows; the low 5 bits of each byte are the columns (bit4 = leftmost dot).
typedef struct { char ch; uint8_t rows[DM_ROWS]; } DMGlyph;

static const DMGlyph DM_FONT[] = {
  {'0', {0b01110,0b10001,0b10011,0b10101,0b11001,0b10001,0b01110}},
  {'1', {0b00100,0b01100,0b00100,0b00100,0b00100,0b00100,0b01110}},
  {'2', {0b01110,0b10001,0b00001,0b00110,0b01000,0b10000,0b11111}},
  {'3', {0b11110,0b00001,0b00001,0b01110,0b00001,0b00001,0b11110}},
  {'4', {0b00010,0b00110,0b01010,0b10010,0b11111,0b00010,0b00010}},
  {'5', {0b11111,0b10000,0b11110,0b00001,0b00001,0b10001,0b01110}},
  {'6', {0b00110,0b01000,0b10000,0b11110,0b10001,0b10001,0b01110}},
  {'7', {0b11111,0b00001,0b00010,0b00100,0b01000,0b01000,0b01000}},
  {'8', {0b01110,0b10001,0b10001,0b01110,0b10001,0b10001,0b01110}},
  {'9', {0b01110,0b10001,0b10001,0b01111,0b00001,0b00010,0b01100}},
  {':', {0b00000,0b00000,0b00100,0b00000,0b00100,0b00000,0b00000}},
  {'/', {0b00001,0b00001,0b00010,0b00100,0b01000,0b10000,0b10000}},
  {'-', {0b00000,0b00000,0b00000,0b11111,0b00000,0b00000,0b00000}},
  {'.', {0b00000,0b00000,0b00000,0b00000,0b00000,0b00000,0b00100}},
  {'%', {0b11001,0b11010,0b00100,0b01011,0b10011,0b00000,0b00000}},
  {' ', {0b00000,0b00000,0b00000,0b00000,0b00000,0b00000,0b00000}},
};
static const uint8_t DM_BLANK[DM_ROWS] = {0,0,0,0,0,0,0};

static const uint8_t *dm_glyph(char c) {
  for (unsigned i = 0; i < sizeof(DM_FONT)/sizeof(DM_FONT[0]); i++) {
    if (DM_FONT[i].ch == c) return DM_FONT[i].rows;
  }
  return DM_BLANK;
}

// Pixel width of a dot-matrix string at the given cell pitch.
static int dm_width(const char *s, int pitch) {
  int n = strlen(s);
  if (n == 0) return 0;
  return (n * DM_ADV - 1) * pitch;   // drop the trailing inter-glyph gap
}

// 4x4 ordered (Bayer) matrix → even density dithering of the ghost grid.
static const uint8_t BAYER4[4][4] = {
  { 0,  8,  2, 10},
  {12,  4, 14,  6},
  { 3, 11,  1,  9},
  {15,  7, 13,  5},
};

// Draw a dot-matrix string. Lit dots use `color`; unlit dots use `ghost`, whose
// prominence is the intensity percentage s_grid (0 = none, 100 = full), applied
// as ordered density so the slider reads smoothly even at small pitch. The
// dither is keyed to absolute dot coordinates so the grid stays stable across
// glyphs and redraws.
static void dm_draw(GContext *ctx, const char *s, int x, int y, int pitch,
                    GColor color, GColor ghost) {
  int dark_r = (pitch * 30) / 100;  if (dark_r < 1) dark_r = 1;
  // Lit dots ALWAYS at least one pixel larger than the ghost, so values stay
  // legible at any grid intensity — including the small date/battery rows
  // where, at pitch 4, lit and ghost would otherwise both be a single pixel.
  int lit_r  = (pitch * 42) / 100;  if (lit_r < dark_r + 1) lit_r = dark_r + 1;
  int cx = x;
  for (const char *p = s; *p; p++) {
    const uint8_t *g = dm_glyph(*p);
    for (int row = 0; row < DM_ROWS; row++) {
      for (int col = 0; col < DM_COLS; col++) {
        bool on = (g[row] >> (DM_COLS - 1 - col)) & 1;
        int dx = cx + col * pitch + pitch / 2;
        int dy = y  + row * pitch + pitch / 2;
        if (!on) {
          // ghost dot shown only where intensity beats this cell's threshold
          int thr = (BAYER4[(dy >> 1) & 3][(dx >> 1) & 3] * 100) / 16;  // 0..93
          if (s_grid <= thr) continue;
        }
        graphics_context_set_fill_color(ctx, on ? color : ghost);
        graphics_fill_circle(ctx, GPoint(dx, dy), on ? lit_r : dark_r);
      }
    }
    cx += DM_ADV * pitch;
  }
}

// ─────────────────────────── steps colour ramp ─────────────────────────────
// Red -> amber -> green, thresholds as fractions of the (configurable) goal.
static GColor step_color(int steps, int goal) {
  if (goal < 1) goal = STEP_GOAL_DEFAULT;
  int pct = (steps <= 0) ? 0 : (steps >= goal ? 100 : (steps * 100) / goal);
  if (pct < 20) return GColorRed;
  if (pct < 40) return GColorOrange;
  if (pct < 65) return GColorChromeYellow;
  if (pct < 90) return GColorBrightGreen;
  return GColorGreen;
}

// ───────────────────────────── label strings ───────────────────────────────
// Drawn in the system Gothic font, so any Latin label is fine. BATT stays a
// compact abbreviation across languages to fit LUMEN's tight rows.
static const char *lbl_steps(void) {
  static const char *t[LANG_COUNT] = {"PAS","STEPS","SCHRITTE","PASOS","PASSI","STAPPEN","PASSOS","KROKI","STEG"};
  return t[LIDX(s_lang)];
}
static const char *lbl_date(void) {
  static const char *t[LANG_COUNT] = {"DATE","DATE","DATUM","FECHA","DATA","DATUM","DATA","DATA","DATUM"};
  return t[LIDX(s_lang)];
}
static const char *lbl_batt(void) { return "BATT"; }

// ──────────────────────────────── render ───────────────────────────────────
static void draw_label(GContext *ctx, const char *txt, int y, int h) {
  graphics_context_set_text_color(ctx, GColorLightGray);
  graphics_draw_text(ctx, txt, fonts_get_system_font(FONT_KEY_GOTHIC_14),
                     GRect(PAD, y, SCREEN_W - 2*PAD, h),
                     GTextOverflowModeFill, GTextAlignmentLeft, NULL);
}

// Right-align a dot-matrix value to the screen's right padding edge.
static void draw_value_right(GContext *ctx, const char *val, int y, int pitch,
                             GColor color, GColor ghost) {
  int w = dm_width(val, pitch);
  int x = SCREEN_W - PAD - w;
  dm_draw(ctx, val, x, y, pitch, color, ghost);
}

static void layer_update(Layer *layer, GContext *ctx) {
  const GColor GHOST = GColorDarkGray;          // ghost (unlit) grid
  const GColor WHITE = GColorWhite;
  const GColor AMBER = GColorChromeYellow;      // ~#F6A91E

  // Near-black LED background.
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(0, 0, SCREEN_W, SCREEN_H), 0, GCornerNone);

  // Live values.
  time_t now = time(NULL);
  struct tm *tm = localtime(&now);
  char timebuf[24], datebuf[24], stepbuf[16], battbuf[16];
  snprintf(timebuf, sizeof(timebuf), "%02d:%02d",
           lumen_disp_hour(tm->tm_hour, s_time24h), tm->tm_min); // steady colon
  lumen_format_date(datebuf, sizeof(datebuf), tm, s_datefmt, s_lang, s_yearfmt);
  snprintf(stepbuf, sizeof(stepbuf), "%d", s_steps);
  snprintf(battbuf, sizeof(battbuf), "%d%%", s_battery);

  // Hero pitch 6 keeps "HH:MM" (29 cells -> 174px) inside the 200px screen
  // with even margins; pitch 7 (203px) overflows and clips the edge digits.
  const int P_SEC  = 4;   // secondary rows (steps/date/batt) cell pitch
  const int P_HERO = 6;   // hero TIME cell pitch
  const int ROW_H  = DM_ROWS * P_SEC;     // 28
  const int LBL_DY = (ROW_H - 14) / 2;    // vertically centre label on its row

  // Four zones, even space-between down the 228px screen.
  const int y_steps = 14;
  const int y_hero  = 66;
  const int y_date  = 132;
  const int y_batt  = 184;

  // STEPS (top) — value in ramp colour.
  draw_label(ctx, lbl_steps(), y_steps + LBL_DY, 16);
  draw_value_right(ctx, stepbuf, y_steps, P_SEC, step_color(s_steps, s_goal), GHOST);

  // TIME (hero, centred) — white, steady colon.
  int hero_w = dm_width(timebuf, P_HERO);
  dm_draw(ctx, timebuf, (SCREEN_W - hero_w) / 2, y_hero, P_HERO, WHITE, GHOST);

  // DATE — amber.
  draw_label(ctx, lbl_date(), y_date + LBL_DY, 16);
  draw_value_right(ctx, datebuf, y_date, P_SEC, AMBER, GHOST);

  // BATTERY (bottom) — white.
  draw_label(ctx, lbl_batt(), y_batt + LBL_DY, 16);
  draw_value_right(ctx, battbuf, y_batt, P_SEC, WHITE, GHOST);

  // Transient shake overlay: full-bleed flash in SYSTEM fonts (NOT dm_draw, so
  // it isn't limited to DM_FONT's numeric glyphs) — auto-cleared by overlay_clear.
  if (s_overlay) {
    GRect b = layer_get_bounds(layer);
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, b, 0, GCornerNone);
    int cy = b.origin.y + b.size.h / 2;
    graphics_context_set_text_color(ctx, s_ov_col);
    graphics_draw_text(ctx, s_ov_main, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD),
      GRect(b.origin.x, cy - 44, b.size.w, 50),
      GTextOverflowModeFill, GTextAlignmentCenter, NULL);
    graphics_context_set_text_color(ctx, GColorLightGray);
    graphics_draw_text(ctx, s_ov_sub, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
      GRect(b.origin.x, cy + 8, b.size.w, 24),
      GTextOverflowModeFill, GTextAlignmentCenter, NULL);
  }
}

// Auto-clears the transient shake overlay after FLIP_OVERLAY_MS.
static void overlay_clear(void *d) {
  (void)d;
  s_overlay = 0;
  if (s_layer) layer_mark_dirty(s_layer);
}

// ──────────────────────────── service handlers ─────────────────────────────
static void tick_handler(struct tm *t, TimeUnits units) {
  layer_mark_dirty(s_layer);   // minute resolution only — no blinking
}

static void battery_handler(BatteryChargeState c) {
  s_battery = c.charge_percent;
  layer_mark_dirty(s_layer);
}

static void health_handler(HealthEventType event, void *ctx) {
  if (event == HealthEventMovementUpdate || event == HealthEventSignificantUpdate) {
    s_steps = (int)health_service_sum_today(HealthMetricStepCount);
    layer_mark_dirty(s_layer);
  }
}

// Shake action is configurable from the phone (default OFF). Language cycling is
// intentionally NOT here — it has its own config menu.
static void tap_handler(AccelAxisType axis, int32_t direction) {
  (void)axis; (void)direction;
  switch (s_shake) {
    case 1:                                              // replay flip (redraw)
      layer_mark_dirty(s_layer);
      break;
    case 2:                                              // backlight pulse
      light_enable_interaction();
      return;                                            // no redraw needed
    case 3: {                                            // steps remaining flash
      int rem = s_goal - s_steps; if (rem < 0) rem = 0;
      snprintf(s_ov_main, sizeof(s_ov_main), "%d", rem);
      strncpy(s_ov_sub, lbl_steps(), sizeof(s_ov_sub) - 1);
      s_ov_sub[sizeof(s_ov_sub) - 1] = 0;
      s_ov_col = step_color(s_steps, s_goal);
      s_overlay = 1;
      app_timer_register(OVERLAY_MS, overlay_clear, NULL);
      break;
    }
    case 4:                                              // random destination
      strncpy(s_ov_main, DEST[rand() % 20], sizeof(s_ov_main) - 1);
      s_ov_main[sizeof(s_ov_main) - 1] = 0;
      strncpy(s_ov_sub, "DEPARTURE", sizeof(s_ov_sub) - 1);
      s_ov_sub[sizeof(s_ov_sub) - 1] = 0;
      s_ov_col = GColorWhite;
      s_overlay = 1;
      app_timer_register(OVERLAY_MS, overlay_clear, NULL);
      break;
    case 5:                                              // toggle 12/24h
      s_time24h = !s_time24h;
      persist_write_bool(PERSIST_T24, s_time24h);
      layer_mark_dirty(s_layer);
      break;
    default:                                             // 0 = Off
      return;
  }
}

// Settings pushed from the phone (Clay). Selects arrive as numeric strings,
// the slider as an int — accept either tuple type.
static void inbox_received(DictionaryIterator *it, void *ctx) {
  bool dirty = false;
  Tuple *l = dict_find(it, MESSAGE_KEY_LANG);
  if (l) {
    int v = (l->type == TUPLE_CSTRING) ? atoi(l->value->cstring) : (int)l->value->int32;
    if (v >= 0 && v < LANG_COUNT) { s_lang = v; persist_write_int(PERSIST_LANG, v); dirty = true; }
  }
  Tuple *g = dict_find(it, MESSAGE_KEY_STEP_GOAL);
  if (g) {
    int v = (g->type == TUPLE_CSTRING) ? atoi(g->value->cstring) : (int)g->value->int32;
    if (v >= 100 && v <= 100000) { s_goal = v; persist_write_int(PERSIST_GOAL, v); dirty = true; }
  }
  Tuple *gr = dict_find(it, MESSAGE_KEY_GRID);
  if (gr) {
    int v = (gr->type == TUPLE_CSTRING) ? atoi(gr->value->cstring) : (int)gr->value->int32;
    if (v >= 0 && v <= 100) { s_grid = v; persist_write_int(PERSIST_GRID, v); dirty = true; }
  }
  Tuple *df = dict_find(it, MESSAGE_KEY_DATE_FMT);
  if (df) {
    int v = (df->type == TUPLE_CSTRING) ? atoi(df->value->cstring) : (int)df->value->int32;
    if (v >= 0 && v <= 4) { s_datefmt = v; persist_write_int(PERSIST_DATEFMT, v); dirty = true; }
  }
  Tuple *yf = dict_find(it, MESSAGE_KEY_YEAR_FMT);
  if (yf) {
    int v = (yf->type == TUPLE_CSTRING) ? atoi(yf->value->cstring) : (int)yf->value->int32;
    if (v >= 0 && v <= 2) { s_yearfmt = v; persist_write_int(PERSIST_YEAR, v); dirty = true; }
  }
  Tuple *sa = dict_find(it, MESSAGE_KEY_SHAKE_ACT);
  if (sa) {
    int v = (sa->type == TUPLE_CSTRING) ? atoi(sa->value->cstring) : (int)sa->value->int32;
    if (v >= 0 && v <= 5) { s_shake = v; persist_write_int(PERSIST_SHAKE, v); dirty = true; }
  }
  if (dirty) layer_mark_dirty(s_layer);
}

// ───────────────────────────── window lifecycle ────────────────────────────
static void window_load(Window *w) {
  Layer *root = window_get_root_layer(w);
  GRect b = layer_get_bounds(root);
  s_layer = layer_create(b);
  layer_set_update_proc(s_layer, layer_update);
  layer_add_child(root, s_layer);
}

static void window_unload(Window *w) {
  layer_destroy(s_layer);
}

static void init(void) {
  if (persist_exists(PERSIST_LANG)) s_lang = persist_read_int(PERSIST_LANG);
  if (persist_exists(PERSIST_GOAL)) s_goal = persist_read_int(PERSIST_GOAL);
  if (persist_exists(PERSIST_GRID)) s_grid = persist_read_int(PERSIST_GRID);
  s_datefmt = persist_exists(PERSIST_DATEFMT) ? persist_read_int(PERSIST_DATEFMT) : 0;
  s_yearfmt = persist_exists(PERSIST_YEAR)    ? persist_read_int(PERSIST_YEAR)    : 0;
  s_shake   = persist_exists(PERSIST_SHAKE)   ? persist_read_int(PERSIST_SHAKE)   : 0;
  s_time24h = persist_exists(PERSIST_T24)     ? persist_read_bool(PERSIST_T24)    : clock_is_24h_style();

  srand((unsigned)time(NULL));

  s_battery = battery_state_service_peek().charge_percent;

  if (health_service_metric_accessible(HealthMetricStepCount,
        time_start_of_today(), time(NULL)) & HealthServiceAccessibilityMaskAvailable) {
    s_steps = (int)health_service_sum_today(HealthMetricStepCount);
  }

  s_window = window_create();
  window_set_background_color(s_window, GColorBlack);
  window_set_window_handlers(s_window, (WindowHandlers){
    .load = window_load, .unload = window_unload });
  window_stack_push(s_window, true);

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  battery_state_service_subscribe(battery_handler);
  health_service_events_subscribe(health_handler, NULL);
  accel_tap_service_subscribe(tap_handler);

  // Phone config channel (Clay → AppMessage). Register before opening.
  app_message_register_inbox_received(inbox_received);
  app_message_open(256, 64);
}

static void deinit(void) {
  accel_tap_service_unsubscribe();
  health_service_events_unsubscribe();
  battery_state_service_unsubscribe();
  tick_timer_service_unsubscribe();
  window_destroy(s_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
