/* Copyright 2020 Naoki Katahira
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include QMK_KEYBOARD_H

// ═════════════════════════════════════════════════════════════════
// GLOBAL CONSTANTS
// ═════════════════════════════════════════════════════════════════

// ── EEPROM ────────────────────────────────────
// Magic byte to validate stored data (blank EEPROM = 0xFF).
#define EEPROM_MAGIC 0xA5
#define EEPROM_WPM_SHIFT 8   // bits 15–8: max WPM
#define EEPROM_WPM_MASK 0xFF // mask to extract the WPM byte

// ── Pomodoro ──────────────────────────────────
#define POMO_WORK_MS (25UL * 60 * 1000) // 25 minutes of work in ms
#define POMO_BREAK_MS (5UL * 60 * 1000) // 5 minutes of break in ms
#define POMO_CYCLES 4                   // cycles per full round

// ── OLED rows (portrait 270°, 5 cols × 16 rows) ──────────────────
#define ROW_LAYER 0
#define ROW_MODS_HEADER 2
#define ROW_MODS_VALUES 3
#define ROW_WPM 5
#define ROW_BEST_WPM 6
#define ROW_WPM_BAR 7
#define ROW_POMODORO 9
#define ROW_POMO_TIMER (ROW_POMODORO + 2) // row 11
#define ROW_POMO_CYCLE (ROW_POMODORO + 4) // row 13

// ── WPM cadence bar ───────────────────────────
#define WPM_MAX 130                            // WPM for a full bar
#define WPM_BAR_CHARS 5                        // number of bar chars
#define WPM_PER_CHAR (WPM_MAX / WPM_BAR_CHARS) // 26 WPM per char

// ── Blink timers ──────────────────────────────
#define BLINK_WPM_MS 300   // blink interval for '+' in WPM bar
#define BLINK_POMO_MS 1000 // blink interval for pomodoro state

// ── Time conversions ──────────────────────────
#define MS_PER_MIN 60000UL
#define MS_PER_SEC 1000UL

// ═════════════════════════════════════════════════════════════════
// LAYERS
// ═════════════════════════════════════════════════════════════════

enum layer_number { _QWERTY = 0, _LOWER, _RAISE, _ADJUST, _MO4, _MO5 };

#define RAISE MO(_RAISE)
#define LOWER MO(_LOWER)

// ═════════════════════════════════════════════════════════════════
// CUSTOM KEYCODES
// ═════════════════════════════════════════════════════════════════

enum custom_keycodes {
    POMO = SAFE_RANGE, // Pomodoro: short=start/pause/resume, long(>=1s)=reset
    WPM_RST,           // Reset WPM record in EEPROM
};

// ═════════════════════════════════════════════════════════════════
// KEYMAPS
// ═════════════════════════════════════════════════════════════════

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

    /* QWERTY
     * ,-----------------------------------------.                    ,-----------------------------------------.
     * | ESC  |   1  |   2  |   3  |   4  |   5  |                    |   6  |   7  |   8  |   9  |   0  |  ~   |
     * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
     * | Tab  |   Q  |   W  |   E  |   R  |   T  |                    |   Y  |   U  |   I  |   O  |   P  |  -   |
     * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
     * |LCTRL |   A  |   S  |   D  |   F  |   G  |-------.    ,-------|   H  |   J  |   K  |   L  |   ;  |  '   |
     * |------+------+------+------+------+------|   [   |    |    ]  |------+------+------+------+------+------|
     * |LShift|   Z  |   X  |   C  |   V  |   B  |-------|    |-------|   N  |   M  |   ,  |   .  |   /  |RShift|
     * `-----------------------------------------/       /     \      \-----------------------------------------'
     *                   |LOWER | LGUI | Alt  | /Space  /       \Enter \  |BackSP| RGUI |RAISE |
     *                   `-------------------''-------'           '------''--------------------'
     */
    [_QWERTY] = LAYOUT(KC_ESC, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_GRV, KC_TAB, KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_MINS, KC_LCTL, KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, KC_SCLN, KC_QUOT, KC_LSFT, KC_Z, KC_X, KC_C, KC_V, KC_B, KC_LBRC, KC_RBRC, KC_N, KC_M, KC_COMM, KC_DOT, KC_SLSH, KC_RSFT, LOWER, KC_LGUI, KC_LALT, KC_SPC, KC_ENT, KC_BSPC, KC_RGUI, RAISE),

    /* LOWER */
    [_LOWER] = LAYOUT(_______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, KC_EXLM, KC_AT, KC_HASH, KC_DLR, KC_PERC, KC_CIRC, KC_AMPR, KC_ASTR, KC_LPRN, KC_RPRN, _______, _______, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, _______, _______, _______, _______, _______, _______, _______, _______, _______, KC_PIPE, KC_GRAVE, KC_PLUS, KC_LCBR, KC_RCBR, _______, _______, _______, _______, _______, _______, _______, _______, _______),

    /* RAISE */
    [_RAISE] = LAYOUT(_______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, KC_F1, KC_F2, KC_F3, KC_F4, KC_F5, KC_F6, KC_F7, KC_F8, KC_F9, KC_F10, KC_F11, KC_F12, _______, _______, _______, _______, _______, _______, XXXXXXX, KC_LEFT, KC_DOWN, KC_UP, KC_RGHT, XXXXXXX, _______, _______, _______, _______, _______, _______, _______, _______, KC_PLUS, KC_EQL, KC_LBRC, KC_RBRC, KC_BSLS, _______, _______, _______, _______, _______, _______, _______, _______, _______),

    /* ADJUST — POMO at first position (ESC in QWERTY).
     * Activated automatically with simultaneous LOWER + RAISE.
     */
    [_ADJUST] = LAYOUT(POMO, WPM_RST, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, _______, _______, _______, _______, _______, _______, _______, _______),

    [_MO4] = LAYOUT(XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, _______, _______, _______, _______, _______, _______, _______, _______),

    [_MO5] = LAYOUT(XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, _______, _______, _______, _______, _______, _______, _______, _______),
};

// ═════════════════════════════════════════════════════════════════
// LAYER STATE
// ═════════════════════════════════════════════════════════════════

layer_state_t layer_state_set_user(layer_state_t state) {
    return update_tri_layer_state(state, _RAISE, _LOWER, _ADJUST);
}

// ═════════════════════════════════════════════════════════════════
// WPM ALL-TIME RECORD (EEPROM)
//
// Stores the highest WPM achieved across sessions.
// Displayed as "H:nnn" on the OLED.
//
// EEPROM layout (uint32_t eeconfig_read/update_user):
//   bits 15–8: max WPM (0–255)    → EEPROM_WPM_SHIFT
//   bits  7–0: EEPROM_MAGIC (0xA5) → validates stored data
// ═════════════════════════════════════════════════════════════════

static uint8_t best_wpm = 0;

// Loads max WPM on power-up.
// If the magic byte does not match (blank EEPROM), best_wpm stays 0.
void keyboard_post_init_user(void) {
    uint32_t data = eeconfig_read_user();
    if ((data & EEPROM_WPM_MASK) == EEPROM_MAGIC) {
        best_wpm = (data >> EEPROM_WPM_SHIFT) & EEPROM_WPM_MASK;
    }
}

// Updates the EEPROM record if the current WPM exceeds it.
// Only writes on a new record to minimize EEPROM wear.
static void wpm_record_update(void) {
    uint8_t w = get_current_wpm();
    if (w > best_wpm) {
        best_wpm = w;
        eeconfig_update_user(EEPROM_MAGIC | ((uint32_t)best_wpm << EEPROM_WPM_SHIFT));
    }
}

// Resets the WPM record to 0 in RAM and EEPROM.
static void wpm_record_reset(void) {
    best_wpm = 0;
    eeconfig_update_user(EEPROM_MAGIC);
}

// ═════════════════════════════════════════════════════════════════
// POMODORO TIMER
//
// Standard Pomodoro technique:
//   - POMO_WORK_MS  of work  (WORK)
//   - POMO_BREAK_MS of break (BREAK)
//   - POMO_CYCLES cycles per round, then resets to 1
//
// States:
//   POMO_STOP  → idle, shows initial time
//   POMO_WORK  → work countdown
//   POMO_BREAK → break countdown
//
// Control: POMO key on ADJUST layer (LOWER + RAISE, ESC position)
//   Short press → start / pause / resume
//   Long press (>= BLINK_POMO_MS) → full reset
// ═════════════════════════════════════════════════════════════════

typedef enum { POMO_STOP = 0, POMO_WORK, POMO_BREAK } pomo_state_t;

static pomo_state_t pomo_state   = POMO_STOP;
static uint32_t     pomo_start   = 0; // timestamp of the start of the current interval
static uint32_t     pomo_paused  = 0; // ms accumulated before pausing
static bool         pomo_running = false;
static uint8_t      pomo_cycle   = 1; // current cycle (1–POMO_CYCLES)

// Returns remaining ms in the current interval.
static uint32_t pomo_remaining(void) {
    uint32_t total   = (pomo_state == POMO_WORK) ? POMO_WORK_MS : POMO_BREAK_MS;
    uint32_t elapsed = pomo_running ? (pomo_paused + timer_elapsed32(pomo_start)) : pomo_paused;
    return elapsed >= total ? 0 : total - elapsed;
}

// Advances to the next state when time reaches 0.
// Called in oled_task_user() to separate logic from render.
static void pomo_tick(void) {
    if (!pomo_running || pomo_remaining() > 0) return;
    pomo_state = (pomo_state == POMO_WORK) ? POMO_BREAK : POMO_WORK;
    if (pomo_state == POMO_WORK) {
        pomo_cycle = (pomo_cycle % POMO_CYCLES) + 1;
    }
    pomo_paused = 0;
    pomo_start  = timer_read32();
}

// Starts the pomodoro from scratch.
static void pomo_start_timer(void) {
    pomo_state   = POMO_WORK;
    pomo_running = true;
    pomo_paused  = 0;
    pomo_start   = timer_read32();
}

// Pauses the pomodoro, accumulating elapsed time.
static void pomo_pause(void) {
    pomo_paused += timer_elapsed32(pomo_start);
    pomo_running = false;
}

// Resumes the pomodoro from where it was paused.
static void pomo_resume(void) {
    pomo_running = true;
    pomo_start   = timer_read32();
}

// Full reset of the pomodoro.
static void pomo_reset(void) {
    pomo_state   = POMO_STOP;
    pomo_running = false;
    pomo_paused  = 0;
    pomo_cycle   = 1;
}

// ═════════════════════════════════════════════════════════════════
// KEY HOOKS
// ═════════════════════════════════════════════════════════════════

// Timestamp to detect long press on POMO.
static uint32_t pomo_hold_start = 0;

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (keycode == WPM_RST && record->event.pressed) {
        wpm_record_reset();
        return false;
    }
    if (keycode == POMO) {
        if (record->event.pressed) {
            // Save timestamp on press to measure duration
            pomo_hold_start = timer_read32();
        } else {
            // On release: long press (>= BLINK_POMO_MS) → reset
            if (timer_elapsed32(pomo_hold_start) >= BLINK_POMO_MS) {
                pomo_reset();
            } else if (pomo_state == POMO_STOP) {
                pomo_start_timer();
            } else if (pomo_running) {
                pomo_pause();
            } else {
                pomo_resume();
            }
        }
        return false; // do not propagate
    }
    if (record->event.pressed) {
        wpm_record_update();
    }
    return true;
}

// ═════════════════════════════════════════════════════════════════
// OLED
//
// Master (left, rotation 270°):
//   Portrait → 32px wide × 128px tall → 5 cols × 16 rows
//   All sections use oled_set_cursor to anchor their row,
//   preventing buffer wrap from overwriting other sections.
//
// Slave (right, rotation 90°):
//   Lily58 logo (tiles 0x80–0xD4 from glcdfont_lily.c)
// ═════════════════════════════════════════════════════════════════

#ifdef OLED_ENABLE

// ── Rotation ─────────────────────────────────
oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    return is_keyboard_master() ? OLED_ROTATION_270 : OLED_ROTATION_90;
}

// ── Slave: Lily58 logo ────────────────────────
// Tiles 0x80–0xD4 from glcdfont_lily.c, 3 rows of 21 chars.
static void render_logo(void) {
    static const char PROGMEM logo[] = {0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F, 0x90, 0x91, 0x92, 0x93, 0x94, 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0x00};
    oled_write_P(logo, false);
}

// ── Active layer ──────────────────────────────
// Format "L:XXX" (5 chars). Numeric fallback for dynamic layers.
static void render_layer(void) {
    oled_set_cursor(0, ROW_LAYER);
    switch (get_highest_layer(layer_state)) {
        case _QWERTY:
            oled_write_P(PSTR("L:QWT"), false);
            break;
        case _LOWER:
            oled_write_P(PSTR("L:LOW"), false);
            break;
        case _RAISE:
            oled_write_P(PSTR("L:RSE"), false);
            break;
        case _ADJUST:
            oled_write_P(PSTR("L:ADJ"), false);
            break;
        default: {
            uint8_t l      = get_highest_layer(layer_state);
            char    buf[6] = {'L', ':', '0' + (l / 10), '0' + (l % 10), ' ', '\0'};
            oled_write(buf, false);
            break;
        }
    }
}

// ── Active modifiers ──────────────────────────
// Header "MODS" + values "SCAG": uppercase=active, dot=inactive.
static void render_mods(void) {
    oled_set_cursor(0, ROW_MODS_HEADER);
    oled_write_P(PSTR("MODS"), false);
    oled_set_cursor(0, ROW_MODS_VALUES);
    uint8_t mods   = get_mods();
    char    str[5] = {(mods & MOD_MASK_SHIFT) ? 'S' : '.', (mods & MOD_MASK_CTRL) ? 'C' : '.', (mods & MOD_MASK_ALT) ? 'A' : '.', (mods & MOD_MASK_GUI) ? 'G' : '.', '\0'};
    oled_write(str, false);
}

// ── Current WPM ───────────────────────────────
// Format "W:nnn" (5 chars). Manual int→char conversion (no sprintf).
static void render_wpm(void) {
    uint8_t w      = get_current_wpm();
    char    str[6] = {'W', ':', '0' + (w / 100), '0' + ((w / 10) % 10), '0' + (w % 10), '\0'};
    oled_set_cursor(0, ROW_WPM);
    oled_write(str, false);
}

// ── All-time WPM record ───────────────────────
// Format "H:nnn" (5 chars). Value loaded from EEPROM at startup.
static void render_best_wpm(void) {
    char str[6] = {'H', ':', '0' + (best_wpm / 100), '0' + ((best_wpm / 10) % 10), '0' + (best_wpm % 10), '\0'};
    oled_set_cursor(0, ROW_BEST_WPM);
    oled_write(str, false);
}

// ── WPM cadence bar ───────────────────────────
// WPM_BAR_CHARS chars proportional to WPM (scale 0–WPM_MAX).
// Partial char blinks '+' every BLINK_WPM_MS when fraction >= 50%.
// Bar inverts (white background) at WPM_MAX.
static uint32_t blink_timer = 0;
static bool     blink_state = false;

static void render_wpm_bar(void) {
    uint8_t w    = get_current_wpm();
    bool    full = w >= WPM_MAX;

    if (timer_elapsed32(blink_timer) > BLINK_WPM_MS) {
        blink_state ^= 1;
        blink_timer = timer_read32();
    }

    uint8_t filled = full ? WPM_BAR_CHARS : (w / WPM_PER_CHAR);
    uint8_t frac   = full ? 0 : (w % WPM_PER_CHAR) * 100 / WPM_PER_CHAR;

    char bar[WPM_BAR_CHARS + 1];
    for (uint8_t i = 0; i < WPM_BAR_CHARS; i++)
        bar[i] = '.';
    bar[WPM_BAR_CHARS] = '\0';
    for (uint8_t i = 0; i < filled; i++)
        bar[i] = '#';
    if (!full && filled < WPM_BAR_CHARS && frac >= 50) {
        bar[filled] = blink_state ? '+' : '.';
    }

    oled_set_cursor(0, ROW_WPM_BAR);
    oled_write(bar, full);
}

// ── Pomodoro timer ────────────────────────────
// State blinks every BLINK_POMO_MS when timer is running.
// Rows anchored: ROW_POMODORO, ROW_POMO_TIMER, ROW_POMO_CYCLE.
static uint32_t pomo_blink_timer = 0;
static bool     pomo_blink_state = false;

static void render_pomodoro(void) {
    if (timer_elapsed32(pomo_blink_timer) > BLINK_POMO_MS) {
        pomo_blink_state ^= 1;
        pomo_blink_timer = timer_read32();
    }

    // State
    bool blink = pomo_running && pomo_blink_state;
    oled_set_cursor(0, ROW_POMODORO);
    switch (pomo_state) {
        case POMO_WORK:
            oled_write_P(PSTR("WORK "), blink);
            break;
        case POMO_BREAK:
            oled_write_P(PSTR("BREAK"), blink);
            break;
        default:
            oled_write_P(PSTR("POMO "), false);
            break;
    }

    // Remaining time MM:SS
    uint32_t rem     = (pomo_state == POMO_STOP) ? POMO_WORK_MS : pomo_remaining();
    uint8_t  minutes = (uint8_t)(rem / MS_PER_MIN);
    uint8_t  seconds = (uint8_t)((rem % MS_PER_MIN) / MS_PER_SEC);
    char     time[6] = {'0' + (minutes / 10), '0' + (minutes % 10), ':', '0' + (seconds / 10), '0' + (seconds % 10), '\0'};
    oled_set_cursor(0, ROW_POMO_TIMER);
    oled_write(time, false);

    // Current cycle / total
    char cyc[6] = {'C', ':', '0' + pomo_cycle, '/', '0' + POMO_CYCLES, '\0'};
    oled_set_cursor(0, ROW_POMO_CYCLE);
    oled_write(cyc, false);
}

// ── Main compositor ───────────────────────────
// Each section uses oled_set_cursor — no wrap between sections.
//
// Row map (portrait 270°, 5 cols × 16 rows):
//  0  L:QRT   active layer
//  1  (empty)
//  2  MODS    modifiers header
//  3  SCAG    modifiers values (uppercase=active, dot=inactive)
//  4  (empty)
//  5  W:nnn   current WPM
//  6  H:nnn   all-time WPM record (EEPROM)
//  7  ##+..   WPM cadence bar (0–WPM_MAX)
//  8  (empty)
//  9  WORK    pomodoro state (blinks every BLINK_POMO_MS when active)
// 10  (empty)
// 11  25:00   remaining time MM:SS
// 12  (empty)
// 13  C:1/4   current cycle / POMO_CYCLES
// 14–15 (empty)
static void render_status(void) {
    render_layer();
    render_mods();
    render_wpm();
    render_best_wpm();
    render_wpm_bar();
    render_pomodoro();
}

// ── OLED driver entry point ───────────────────
// pomo_tick() is called here (advance logic) before rendering.
bool oled_task_user(void) {
    pomo_tick();
    if (is_keyboard_master())
        render_status();
    else
        render_logo();
    return false;
}

#endif // OLED_ENABLE
