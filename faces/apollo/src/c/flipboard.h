// flipboard.h — shared engine for the FlipBoard watchface family (Option A:
// one .pbw per face, each #includes this header). Provides the common data
// model, the steps colour ramp, bilingual labels, and a window+services
// runtime that calls a per-face render callback. Each face's main.c is just a
// render function + main(){ flip_run(...); }.
//
// This header is COPIED into each face's src/c/ at build time (keep the canonical
// copy in FlipBoard/shared/ and re-sync after edits).

#pragma once
#include <pebble.h>

#define FLIP_STEP_GOAL_DEFAULT 10000
#define FLIP_STEP_GOAL FLIP_STEP_GOAL_DEFAULT   // compat alias for faces not yet migrated
#define FLIP_W 200
#define FLIP_H 228
#define FLIP_PERSIST_LANG 1
#define FLIP_PERSIST_GOAL 2
#define FLIP_PERSIST_DATEFMT 3
#define FLIP_PERSIST_SHAKE   4
#define FLIP_PERSIST_T24     5

// Date display formats (Clay select value == enum). AUTO (0) follows the
// selected language; the rest are manual overrides.
enum { FLIP_DF_AUTO = 0, FLIP_DF_DMY = 1, FLIP_DF_MDY = 2, FLIP_DF_DOT = 3, FLIP_DF_ISO = 4 };
// Shake actions (Clay select value == enum). OFF is the default (no accidental
// triggers while walking). Language cycling lives in its own menu, not here.
enum { FLIP_SH_OFF = 0, FLIP_SH_FLIP = 1, FLIP_SH_LIGHT = 2,
       FLIP_SH_STEPSLEFT = 3, FLIP_SH_DEST = 4, FLIP_SH_T24 = 5 };
#define FLIP_OVERLAY_MS 3500

// Languages selectable from the phone config (Clay) and by shake-to-cycle.
// Latin-script only — the watch's baked-sprite + system-font pipeline can't
// render Cyrillic/Greek/CJK without tofu. The enum value IS the persisted code
// and the Clay option value; keep FLIP_FR=0 / FLIP_EN=1 stable for back-compat.
enum { FLIP_FR = 0, FLIP_EN = 1, FLIP_DE = 2, FLIP_ES = 3, FLIP_IT = 4,
       FLIP_NL = 5, FLIP_PT = 6, FLIP_PL = 7, FLIP_SV = 8 };
#define FLIP_LANG_COUNT 9

typedef struct {
  int steps;
  int battery;
  int lang;       // FLIP_FR..FLIP_SV
  int step_goal;  // daily step goal — configurable from the phone, persisted
  int anim;       // split-flap frames remaining after a minute tick (FLIP_ANIM_FRAMES..0)
  int date_fmt;   // FLIP_DF_* — date display format (phone config)
  int shake_act;  // FLIP_SH_* — what a wrist shake does (phone config)
  bool time_24h;  // 24h vs 12h time; default = system; toggled by shake
  // Transient shake overlay (steps-left / destination), drawn by the engine
  // over the face in a system font so it works even on segment-only faces.
  int overlay;    // 0 none, 1 steps-left, 2 destination
  char ov_main[16];
  char ov_sub[16];
  GColor ov_col;
  GColor bg;      // face background (for overlay fill)
} FlipState;

#define FLIP_ANIM_FRAMES 5
#define FLIP_ANIM_MS     70

// ───────────────────────── steps colour ramp ─────────────────────────
// 5-stop device-accurate quantisation of the handoff red→amber→green HSL.
// Stops chosen by e-paper display calibration (qemu emery LUT, 2026-06-10):
// the panel desaturates heavily, so stops are picked by their DISPLAYED value.
// Red near 0 steps, green as you approach the goal. Thresholds are fractions
// of the (configurable) goal so the ramp tracks whatever target the user sets.
static inline GColor flip_step_color(int s, int goal) {
  if (goal < 1) goal = FLIP_STEP_GOAL_DEFAULT;
  int pct = (s <= 0) ? 0 : (s >= goal ? 100 : (s * 100) / goal);
  if (pct < 20) return GColorSunsetOrange;            // shows warm red
  if (pct < 40) return GColorChromeYellow;            // shows amber-orange
  if (pct < 65) return GColorYellow;                  // shows pale yellow
  if (pct < 90) return GColorFromRGB(85, 255, 0);     // shows light green
  return GColorGreen;                                 // shows full green
}
// Ramp variant for LIGHT backgrounds (e.g. IVOIRE): darker, saturated tones
// that stay legible on cream/ivory where bright yellow/green would wash out.
static inline GColor flip_step_color_dark(int s, int goal) {
  if (goal < 1) goal = FLIP_STEP_GOAL_DEFAULT;
  int pct = (s <= 0) ? 0 : (s >= goal ? 100 : (s * 100) / goal);
  if (pct < 20) return GColorRed;
  if (pct < 40) return GColorWindsorTan;
  if (pct < 65) return GColorLimerick;
  if (pct < 90) return GColorIslamicGreen;
  return GColorDarkGreen;
}
static inline int flip_progress_pct(int s, int goal) {
  if (goal < 1) goal = FLIP_STEP_GOAL_DEFAULT;
  if (s < 0) s = 0;
  if (s > goal) s = goal;
  return (s * 100) / goal;
}

// ───────────────────────── localised labels ─────────────────────────
// Tables indexed by language code (ASCII uppercase to avoid font tofu).
#define FLIP_L(lang) (((lang) >= 0 && (lang) < FLIP_LANG_COUNT) ? (lang) : 0)
static inline const char *flip_lbl_steps(int lang) {
  static const char *t[FLIP_LANG_COUNT] = {"PAS","STEPS","SCHRITTE","PASOS","PASSI","STAPPEN","PASSOS","KROKI","STEG"};
  return t[FLIP_L(lang)];
}
static inline const char *flip_lbl_time(int lang) {
  static const char *t[FLIP_LANG_COUNT] = {"HEURE","TIME","ZEIT","HORA","ORA","TIJD","HORA","CZAS","TID"};
  return t[FLIP_L(lang)];
}
static inline const char *flip_lbl_date(int lang) {
  static const char *t[FLIP_LANG_COUNT] = {"DATE","DATE","DATUM","FECHA","DATA","DATUM","DATA","DATA","DATUM"};
  return t[FLIP_L(lang)];
}
static inline const char *flip_lbl_batt(int lang) {
  static const char *t[FLIP_LANG_COUNT] = {"BATTERIE","BATTERY","AKKU","BATERIA","BATTERIA","ACCU","BATERIA","BATERIA","BATTERI"};
  return t[FLIP_L(lang)];
}
static inline const char *flip_lbl_batt_short(int lang) { (void)lang; return "BATT"; }
static inline const char *flip_weekday(int lang, int wday) {
  static const char *wd[FLIP_LANG_COUNT][7] = {
    {"DIM","LUN","MAR","MER","JEU","VEN","SAM"}, // FR
    {"SUN","MON","TUE","WED","THU","FRI","SAT"}, // EN
    {"SO","MO","DI","MI","DO","FR","SA"},        // DE
    {"DOM","LUN","MAR","MIE","JUE","VIE","SAB"}, // ES
    {"DOM","LUN","MAR","MER","GIO","VEN","SAB"}, // IT
    {"ZO","MA","DI","WO","DO","VR","ZA"},        // NL
    {"DOM","SEG","TER","QUA","QUI","SEX","SAB"}, // PT
    {"NIE","PON","WTO","SRO","CZW","PIA","SOB"}, // PL
    {"SON","MAN","TIS","ONS","TOR","FRE","LOR"}, // SV
  };
  if (wday < 0 || wday > 6) wday = 0;
  return wd[FLIP_L(lang)][wday];
}

// ───────────────────────── date / time format ─────────────────────────
// The conventional numeric date order/separator for each language.
static inline int flip_lang_date_fmt(int lang) {
  switch (FLIP_L(lang)) {
    case FLIP_EN: return FLIP_DF_MDY;                       // US month-first
    case FLIP_DE: case FLIP_NL: case FLIP_PL: return FLIP_DF_DOT;  // 29.06
    case FLIP_SV: return FLIP_DF_ISO;                       // Sweden uses ISO
    default:      return FLIP_DF_DMY;                       // FR, ES, IT, PT
  }
}
// Numeric-only formats so every face (incl. 7-segment QUAI) can render them.
// fmt FLIP_DF_AUTO resolves to the language's convention; otherwise it's manual.
static inline void flip_format_date(char *buf, size_t n, struct tm *now, int fmt, int lang) {
  if (fmt == FLIP_DF_AUTO) fmt = flip_lang_date_fmt(lang);
  int d = now->tm_mday, m = now->tm_mon + 1, y = now->tm_year + 1900;
  switch (fmt) {
    case FLIP_DF_MDY: snprintf(buf, n, "%02d/%02d", m, d); break;
    case FLIP_DF_DOT: snprintf(buf, n, "%02d.%02d", d, m); break;
    case FLIP_DF_ISO: snprintf(buf, n, "%04d-%02d-%02d", y, m, d); break;
    default:          snprintf(buf, n, "%02d/%02d", d, m); break;  // DMY
  }
}
// Hour to display, honouring the 12/24h setting (no AM/PM — space is tight).
static inline int flip_disp_hour(int hour24, bool h24) {
  if (h24) return hour24;
  int h = hour24 % 12;
  return h == 0 ? 12 : h;
}

// Departure-board easter egg: a shake can flash a random destination code.
static const char *FLIP_DEST[] = {
  "CDG","JFK","LHR","HND","SFO","DXB","SIN","AMS","FRA","NRT",
  "LAX","HKG","GVA","BCN","FCO","SYD","YUL","GRU","ICN","MAD" };
#define FLIP_DEST_COUNT 20

// ───────────────────────── engine runtime ─────────────────────────
// A face provides a render callback and an optional setup callback (load custom
// fonts before the first paint). The render receives `area` = the current
// UNOBSTRUCTED bounds (full screen normally; the top slice during Quick View /
// timeline peek). Faces must lay out inside `area` so nothing hides under the
// Quick View overlay.
typedef void (*FlipRender)(GContext *ctx, const FlipState *st, struct tm *now, GRect area);
typedef void (*FlipSetup)(void);

static Window     *s_flip_window;
static Layer      *s_flip_layer;
static FlipState   s_flip;
static FlipRender  s_flip_render;

static void s_flip_update(Layer *layer, GContext *ctx) {
  time_t t = time(NULL);
  struct tm *now = localtime(&t);
  GRect area = layer_get_unobstructed_bounds(layer);   // Quick View aware
  s_flip_render(ctx, &s_flip, now, area);
  // Transient shake overlay: full-bleed flash in system fonts (works on every
  // face, including segment-only ones) — auto-cleared by s_flip_ov_cb.
  if (s_flip.overlay) {
    graphics_context_set_fill_color(ctx, s_flip.bg);
    graphics_fill_rect(ctx, area, 0, GCornerNone);
    GFont fbig = fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD);
    GFont fsub = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
    int cy = area.origin.y + area.size.h / 2;
    graphics_context_set_text_color(ctx, s_flip.ov_col);
    graphics_draw_text(ctx, s_flip.ov_main, fbig,
      GRect(area.origin.x, cy - 44, area.size.w, 50),
      GTextOverflowModeFill, GTextAlignmentCenter, NULL);
    graphics_context_set_text_color(ctx, GColorLightGray);
    graphics_draw_text(ctx, s_flip.ov_sub, fsub,
      GRect(area.origin.x, cy + 8, area.size.w, 24),
      GTextOverflowModeFill, GTextAlignmentCenter, NULL);
  }
}
static void s_flip_ov_cb(void *d) {
  (void)d;
  s_flip.overlay = 0;
  if (s_flip_layer) layer_mark_dirty(s_flip_layer);
}
static void s_flip_ua_changed(AnimationProgress p, void *ctx) {
  (void)p; (void)ctx;
  layer_mark_dirty(s_flip_layer);   // reflow as the Quick View overlay slides
}
// Minute tick starts a short split-flap animation: FLIP_ANIM_FRAMES quick
// redraws (~350 ms total, once per minute — negligible battery cost). Faces
// read st->anim and draw a folding shutter on the digits that changed.
static void s_flip_anim_cb(void *d) {
  (void)d;
  if (s_flip.anim > 0) {
    s_flip.anim--;
    layer_mark_dirty(s_flip_layer);
    if (s_flip.anim > 0)
      app_timer_register(FLIP_ANIM_MS, s_flip_anim_cb, NULL);
  }
}
static void s_flip_tick(struct tm *t, TimeUnits u) {
  (void)t; (void)u;
  s_flip.anim = FLIP_ANIM_FRAMES;
  app_timer_register(FLIP_ANIM_MS, s_flip_anim_cb, NULL);
  layer_mark_dirty(s_flip_layer);
}
static void s_flip_batt(BatteryChargeState c) { s_flip.battery = c.charge_percent; layer_mark_dirty(s_flip_layer); }
static void s_flip_health(HealthEventType e, void *ctx) {
  (void)ctx;
  if (e == HealthEventMovementUpdate || e == HealthEventSignificantUpdate) {
    int steps = (int)health_service_sum_today(HealthMetricStepCount);
    s_flip.steps = (steps >= 0 && steps <= 99999) ? steps : 0;
    layer_mark_dirty(s_flip_layer);
  }
}
// Shake action is configurable from the phone (default OFF). Language cycling
// is intentionally NOT here — it has its own config menu.
static void s_flip_tap(AccelAxisType axis, int32_t direction) {
  (void)axis; (void)direction;
  switch (s_flip.shake_act) {
    case FLIP_SH_FLIP:                                   // replay the split-flap
      s_flip.anim = FLIP_ANIM_FRAMES;
      app_timer_register(FLIP_ANIM_MS, s_flip_anim_cb, NULL);
      break;
    case FLIP_SH_LIGHT:                                  // backlight pulse
      light_enable_interaction();
      return;                                            // no redraw needed
    case FLIP_SH_STEPSLEFT: {                            // steps remaining flash
      int rem = s_flip.step_goal - s_flip.steps; if (rem < 0) rem = 0;
      snprintf(s_flip.ov_main, sizeof s_flip.ov_main, "%d", rem);
      strncpy(s_flip.ov_sub, flip_lbl_steps(s_flip.lang), sizeof s_flip.ov_sub - 1);
      s_flip.ov_sub[sizeof s_flip.ov_sub - 1] = 0;
      s_flip.ov_col = flip_step_color(s_flip.steps, s_flip.step_goal);
      s_flip.overlay = 1;
      app_timer_register(FLIP_OVERLAY_MS, s_flip_ov_cb, NULL);
      break;
    }
    case FLIP_SH_DEST: {                                 // random destination
      strncpy(s_flip.ov_main, FLIP_DEST[rand() % FLIP_DEST_COUNT], sizeof s_flip.ov_main - 1);
      s_flip.ov_main[sizeof s_flip.ov_main - 1] = 0;
      strncpy(s_flip.ov_sub, "DEPARTURE", sizeof s_flip.ov_sub - 1);
      s_flip.ov_sub[sizeof s_flip.ov_sub - 1] = 0;
      s_flip.ov_col = GColorWhite;
      s_flip.overlay = 2;
      app_timer_register(FLIP_OVERLAY_MS, s_flip_ov_cb, NULL);
      break;
    }
    case FLIP_SH_T24:                                    // toggle 12/24h
      s_flip.time_24h = !s_flip.time_24h;
      persist_write_bool(FLIP_PERSIST_T24, s_flip.time_24h);
      break;
    default:                                             // FLIP_SH_OFF
      return;
  }
  layer_mark_dirty(s_flip_layer);
}
// messageKeys declared in package.json are emitted by the build as runtime
// uint32_t symbols (build/src/message_keys.auto.c); the auto header doesn't
// extern them, so declare them here for the inbox handler.
extern uint32_t MESSAGE_KEY_LANG;
extern uint32_t MESSAGE_KEY_STEP_GOAL;
extern uint32_t MESSAGE_KEY_DATE_FMT;
extern uint32_t MESSAGE_KEY_SHAKE_ACT;

// Settings pushed from the phone (Clay). Clay sends a Select as a numeric
// string and a Slider as an int, so accept either tuple type.
static void s_flip_inbox(DictionaryIterator *it, void *ctx) {
  (void)ctx;
  bool dirty = false;
  Tuple *l = dict_find(it, MESSAGE_KEY_LANG);
  if (l) {
    int v = (l->type == TUPLE_CSTRING) ? atoi(l->value->cstring) : (int)l->value->int32;
    if (v >= 0 && v < FLIP_LANG_COUNT) { s_flip.lang = v; persist_write_int(FLIP_PERSIST_LANG, v); dirty = true; }
  }
  Tuple *g = dict_find(it, MESSAGE_KEY_STEP_GOAL);
  if (g) {
    int v = (g->type == TUPLE_CSTRING) ? atoi(g->value->cstring) : (int)g->value->int32;
    if (v >= 100 && v <= 100000) { s_flip.step_goal = v; persist_write_int(FLIP_PERSIST_GOAL, v); dirty = true; }
  }
  Tuple *df = dict_find(it, MESSAGE_KEY_DATE_FMT);
  if (df) {
    int v = (df->type == TUPLE_CSTRING) ? atoi(df->value->cstring) : (int)df->value->int32;
    if (v >= 0 && v <= FLIP_DF_ISO) { s_flip.date_fmt = v; persist_write_int(FLIP_PERSIST_DATEFMT, v); dirty = true; }
  }
  Tuple *sa = dict_find(it, MESSAGE_KEY_SHAKE_ACT);
  if (sa) {
    int v = (sa->type == TUPLE_CSTRING) ? atoi(sa->value->cstring) : (int)sa->value->int32;
    if (v >= 0 && v <= FLIP_SH_T24) { s_flip.shake_act = v; persist_write_int(FLIP_PERSIST_SHAKE, v); dirty = true; }
  }
  if (dirty) layer_mark_dirty(s_flip_layer);
}
static void s_flip_load(Window *w) {
  Layer *root = window_get_root_layer(w);
  s_flip_layer = layer_create(layer_get_bounds(root));
  layer_set_update_proc(s_flip_layer, s_flip_update);
  layer_add_child(root, s_flip_layer);
}
static void s_flip_unload(Window *w) { (void)w; layer_destroy(s_flip_layer); }

// Run the watchface. `render` is mandatory; `setup` may be NULL.
static void flip_run(FlipRender render, GColor bg, FlipSetup setup) {
  s_flip_render = render;
  s_flip.lang = persist_exists(FLIP_PERSIST_LANG) ? persist_read_int(FLIP_PERSIST_LANG) : FLIP_FR;
  s_flip.step_goal = persist_exists(FLIP_PERSIST_GOAL) ? persist_read_int(FLIP_PERSIST_GOAL) : FLIP_STEP_GOAL_DEFAULT;
  s_flip.date_fmt = persist_exists(FLIP_PERSIST_DATEFMT) ? persist_read_int(FLIP_PERSIST_DATEFMT) : FLIP_DF_AUTO;
  s_flip.shake_act = persist_exists(FLIP_PERSIST_SHAKE) ? persist_read_int(FLIP_PERSIST_SHAKE) : FLIP_SH_OFF;
  s_flip.time_24h = persist_exists(FLIP_PERSIST_T24) ? persist_read_bool(FLIP_PERSIST_T24) : clock_is_24h_style();
  s_flip.bg = bg;
  s_flip.overlay = 0;
  srand((unsigned)time(NULL));
  s_flip.battery = battery_state_service_peek().charge_percent;
  s_flip.steps = 0;
  if (health_service_metric_accessible(HealthMetricStepCount,
        time_start_of_today(), time(NULL)) & HealthServiceAccessibilityMaskAvailable) {
    int steps = (int)health_service_sum_today(HealthMetricStepCount);
    s_flip.steps = (steps >= 0 && steps <= 99999) ? steps : 0;
  }
  if (setup) setup();

  s_flip_window = window_create();
  window_set_background_color(s_flip_window, bg);
  window_set_window_handlers(s_flip_window, (WindowHandlers){ .load = s_flip_load, .unload = s_flip_unload });
  window_stack_push(s_flip_window, true);

  tick_timer_service_subscribe(MINUTE_UNIT, s_flip_tick);
  battery_state_service_subscribe(s_flip_batt);
  health_service_events_subscribe(s_flip_health, NULL);
  accel_tap_service_subscribe(s_flip_tap);
  unobstructed_area_service_subscribe(
    (UnobstructedAreaHandlers){ .change = s_flip_ua_changed }, NULL);

  // Phone config channel (Clay → AppMessage). Register before opening.
  app_message_register_inbox_received(s_flip_inbox);
  app_message_open(256, 64);

  app_event_loop();

  unobstructed_area_service_unsubscribe();
  accel_tap_service_unsubscribe();
  health_service_events_unsubscribe();
  battery_state_service_unsubscribe();
  tick_timer_service_unsubscribe();
  window_destroy(s_flip_window);
}
