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
// Shake cycles to the next language (quick on-wrist switch); the phone config
// sets it directly. Both paths persist so the choice survives a reload.
static void s_flip_tap(AccelAxisType axis, int32_t direction) {
  (void)axis; (void)direction;
  s_flip.lang = (s_flip.lang + 1) % FLIP_LANG_COUNT;
  persist_write_int(FLIP_PERSIST_LANG, s_flip.lang);
  layer_mark_dirty(s_flip_layer);
}
// messageKeys declared in package.json are emitted by the build as runtime
// uint32_t symbols (build/src/message_keys.auto.c); the auto header doesn't
// extern them, so declare them here for the inbox handler.
extern uint32_t MESSAGE_KEY_LANG;
extern uint32_t MESSAGE_KEY_STEP_GOAL;

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
