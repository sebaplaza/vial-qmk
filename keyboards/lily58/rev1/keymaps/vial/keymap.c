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
// CONSTANTES GLOBALES
// ═════════════════════════════════════════════════════════════════

// ── EEPROM ────────────────────────────────────
// Magic byte para validar que hay datos grabados (EEPROM virgen = 0xFF).
#define EEPROM_MAGIC 0xA5
#define EEPROM_WPM_SHIFT 8   // bits 15–8 : WPM máximo
#define EEPROM_WPM_MASK 0xFF // máscara para extraer el byte de WPM

// ── Pomodoro ──────────────────────────────────
#define POMO_WORK_MS (25UL * 60 * 1000) // 25 minutos de trabajo en ms
#define POMO_BREAK_MS (5UL * 60 * 1000) // 5 minutos de pausa en ms
#define POMO_CYCLES 4                   // ciclos por ronda completa

#define ROW_LAYER 0
#define ROW_MODS_HEADER 2
#define ROW_MODS_VALUES 3
#define ROW_WPM 5
#define ROW_BEST_WPM 6
#define ROW_WPM_BAR 7
#define ROW_POMODORO 9
#define ROW_POMO_TIMER (ROW_POMODORO + 2) // fila 11
#define ROW_POMO_CYCLE (ROW_POMODORO + 4) // fila 13

// ── Barra de cadencia WPM ─────────────────────
#define WPM_MAX 130                            // WPM para barra completamente llena
#define WPM_BAR_CHARS 5                        // número de chars de la barra
#define WPM_PER_CHAR (WPM_MAX / WPM_BAR_CHARS) // 26 WPM por char

// ── Timers de parpadeo ────────────────────────
#define BLINK_WPM_MS 300   // intervalo parpadeo '+' en barra WPM
#define BLINK_POMO_MS 1000 // intervalo parpadeo estado pomodoro

// ── Conversiones de tiempo ────────────────────
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
    POMO = SAFE_RANGE, // Pomodoro: corta=inicia/pausa/reanuda, larga(≥1s)=reset
    WPM_RST,           // Reset del récord de WPM en EEPROM
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

    /* ADJUST — POMO en primera posición (ESC en QWERTY).
     * Activado automáticamente con LOWER + RAISE simultáneo.
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
// WPM MÁXIMO HISTÓRICO (EEPROM)
//
// Guarda el WPM máximo alcanzado entre sesiones.
// Se muestra como "H:nnn" en el OLED.
//
// Layout EEPROM (uint32_t eeconfig_read/update_user):
//   bits 15–8 : WPM máximo (0–255)       → EEPROM_WPM_SHIFT
//   bits  7–0 : EEPROM_MAGIC (0xA5)      → valida que hay datos grabados
// ═════════════════════════════════════════════════════════════════

static uint8_t best_wpm = 0;

// Carga el WPM máximo al encender.
// Si el magic byte no coincide (EEPROM virgen), best_wpm queda en 0.
void keyboard_post_init_user(void) {
    uint32_t data = eeconfig_read_user();
    if ((data & EEPROM_WPM_MASK) == EEPROM_MAGIC) {
        best_wpm = (data >> EEPROM_WPM_SHIFT) & EEPROM_WPM_MASK;
    }
}

// Actualiza el récord en EEPROM si el WPM actual lo supera.
// Solo escribe cuando hay nuevo récord para minimizar el desgaste.
static void wpm_record_update(void) {
    uint8_t w = get_current_wpm();
    if (w > best_wpm) {
        best_wpm = w;
        eeconfig_update_user(EEPROM_MAGIC | ((uint32_t)best_wpm << EEPROM_WPM_SHIFT));
    }
}

// Resetea el récord de WPM a 0 en RAM y en EEPROM.
static void wpm_record_reset(void) {
    best_wpm = 0;
    eeconfig_update_user(EEPROM_MAGIC);
}

// ═════════════════════════════════════════════════════════════════
// POMODORO TIMER
//
// Técnica Pomodoro estándar:
//   - POMO_WORK_MS  de trabajo (WORK)
//   - POMO_BREAK_MS de pausa   (BREAK)
//   - POMO_CYCLES ciclos por ronda, luego vuelve a 1
//
// Estados:
//   POMO_STOP  → inactivo, muestra tiempo inicial
//   POMO_WORK  → cuenta regresiva de trabajo
//   POMO_BREAK → cuenta regresiva de pausa
//
// Control: tecla POMO en capa ADJUST (LOWER + RAISE, posición ESC)
//   Pulsación corta → inicia / pausa / reanuda
//   Pulsación larga (≥ BLINK_POMO_MS) → reset completo
// ═════════════════════════════════════════════════════════════════

typedef enum { POMO_STOP = 0, POMO_WORK, POMO_BREAK } pomo_state_t;

static pomo_state_t pomo_state   = POMO_STOP;
static uint32_t     pomo_start   = 0; // timestamp del inicio del intervalo actual
static uint32_t     pomo_paused  = 0; // ms acumulados antes de pausar
static bool         pomo_running = false;
static uint8_t      pomo_cycle   = 1; // ciclo actual (1–POMO_CYCLES)

// Devuelve los ms restantes del intervalo actual.
static uint32_t pomo_remaining(void) {
    uint32_t total   = (pomo_state == POMO_WORK) ? POMO_WORK_MS : POMO_BREAK_MS;
    uint32_t elapsed = pomo_running ? (pomo_paused + timer_elapsed32(pomo_start)) : pomo_paused;
    return elapsed >= total ? 0 : total - elapsed;
}

// Avanza al siguiente estado cuando el tiempo llega a 0.
// Llamado en oled_task_user() para separar lógica de render.
static void pomo_tick(void) {
    if (!pomo_running || pomo_remaining() > 0) return;
    pomo_state = (pomo_state == POMO_WORK) ? POMO_BREAK : POMO_WORK;
    if (pomo_state == POMO_WORK) {
        pomo_cycle = (pomo_cycle % POMO_CYCLES) + 1;
    }
    pomo_paused = 0;
    pomo_start  = timer_read32();
}

// Inicia el pomodoro desde cero.
static void pomo_start_timer(void) {
    pomo_state   = POMO_WORK;
    pomo_running = true;
    pomo_paused  = 0;
    pomo_start   = timer_read32();
}

// Pausa el pomodoro acumulando el tiempo transcurrido.
static void pomo_pause(void) {
    pomo_paused += timer_elapsed32(pomo_start);
    pomo_running = false;
}

// Reanuda el pomodoro desde donde se pausó.
static void pomo_resume(void) {
    pomo_running = true;
    pomo_start   = timer_read32();
}

// Reset completo del pomodoro.
static void pomo_reset(void) {
    pomo_state   = POMO_STOP;
    pomo_running = false;
    pomo_paused  = 0;
    pomo_cycle   = 1;
}

// ═════════════════════════════════════════════════════════════════
// HOOKS DE TECLAS
// ═════════════════════════════════════════════════════════════════

// Timestamp para detectar pulsación larga de POMO.
static uint32_t pomo_hold_start = 0;

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (keycode == WPM_RST && record->event.pressed) {
        wpm_record_reset();
        return false;
    }
    if (keycode == POMO) {
        if (record->event.pressed) {
            // Guardar timestamp al presionar para medir duración
            pomo_hold_start = timer_read32();
        } else {
            // Al soltar: pulsación larga (≥ BLINK_POMO_MS) → reset
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
        return false; // no propagar
    }
    if (record->event.pressed) {
        wpm_record_update();
    }
    return true;
}

// ═════════════════════════════════════════════════════════════════
// OLED
//
// Master (izquierdo, rotación 270°):
//   Portrait → 32px ancho × 128px alto → 5 cols × 16 filas
//   Todas las secciones usan oled_set_cursor para anclar su fila,
//   evitando que el wrap del buffer sobreescriba otras secciones.
//
// Slave (derecho, rotación 90°):
//   Logo Lily58 (tiles 0x80–0xD4 del glcdfont_lily.c)
// ═════════════════════════════════════════════════════════════════

#ifdef OLED_ENABLE

// ── Rotación ─────────────────────────────────
oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    return is_keyboard_master() ? OLED_ROTATION_270 : OLED_ROTATION_90;
}

// ── Slave: logo Lily58 ────────────────────────
// Tiles 0x80–0xD4 del glcdfont_lily.c, 3 filas de 21 chars.
static void render_logo(void) {
    static const char PROGMEM logo[] = {0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F, 0x90, 0x91, 0x92, 0x93, 0x94, 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0x00};
    oled_write_P(logo, false);
}

// ── Layer activo ──────────────────────────────
// Formato "L:XXX" (5 chars). Fallback numérico para layers dinámicos.
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

// ── Modificadores activos ─────────────────────
// Header "MODS" + valores "SCAG": mayúscula=activo, punto=inactivo.
static void render_mods(void) {
    oled_set_cursor(0, ROW_MODS_HEADER);
    oled_write_P(PSTR("MODS"), false);
    oled_set_cursor(0, ROW_MODS_VALUES);
    uint8_t mods   = get_mods();
    char    str[5] = {(mods & MOD_MASK_SHIFT) ? 'S' : '.', (mods & MOD_MASK_CTRL) ? 'C' : '.', (mods & MOD_MASK_ALT) ? 'A' : '.', (mods & MOD_MASK_GUI) ? 'G' : '.', '\0'};
    oled_write(str, false);
}

// ── WPM actual ────────────────────────────────
// Formato "W:nnn" (5 chars). Conversión manual int→char (sin sprintf).
static void render_wpm(void) {
    uint8_t w      = get_current_wpm();
    char    str[6] = {'W', ':', '0' + (w / 100), '0' + ((w / 10) % 10), '0' + (w % 10), '\0'};
    oled_set_cursor(0, ROW_WPM);
    oled_write(str, false);
}

// ── WPM máximo histórico ──────────────────────
// Formato "H:nnn" (5 chars). Valor cargado desde EEPROM al inicio.
static void render_best_wpm(void) {
    char str[6] = {'H', ':', '0' + (best_wpm / 100), '0' + ((best_wpm / 10) % 10), '0' + (best_wpm % 10), '\0'};
    oled_set_cursor(0, ROW_BEST_WPM);
    oled_write(str, false);
}

// ── Barra de cadencia WPM ─────────────────────
// WPM_BAR_CHARS chars proporcionales al WPM (escala 0–WPM_MAX).
// El char parcial parpadea con '+' cada BLINK_WPM_MS cuando fracción ≥ 50%.
// La barra se invierte (fondo blanco) al alcanzar WPM_MAX.
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
// Estado parpadea cada BLINK_POMO_MS cuando el timer está en marcha.
// Filas ancladas: ROW_POMODORO, ROW_POMO_TIMER, ROW_POMO_CYCLE.
static uint32_t pomo_blink_timer = 0;
static bool     pomo_blink_state = false;

static void render_pomodoro(void) {
    if (timer_elapsed32(pomo_blink_timer) > BLINK_POMO_MS) {
        pomo_blink_state ^= 1;
        pomo_blink_timer = timer_read32();
    }

    // Estado
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

    // Tiempo restante MM:SS
    uint32_t rem     = (pomo_state == POMO_STOP) ? POMO_WORK_MS : pomo_remaining();
    uint8_t  minutes = (uint8_t)(rem / MS_PER_MIN);
    uint8_t  seconds = (uint8_t)((rem % MS_PER_MIN) / MS_PER_SEC);
    char     time[6] = {'0' + (minutes / 10), '0' + (minutes % 10), ':', '0' + (seconds / 10), '0' + (seconds % 10), '\0'};
    oled_set_cursor(0, ROW_POMO_TIMER);
    oled_write(time, false);

    // Ciclo actual / total
    char cyc[6] = {'C', ':', '0' + pomo_cycle, '/', '0' + POMO_CYCLES, '\0'};
    oled_set_cursor(0, ROW_POMO_CYCLE);
    oled_write(cyc, false);
}

// ── Compositor principal ──────────────────────
// Cada sección usa oled_set_cursor — no hay wrap entre secciones.
//
// Mapa de filas (portrait 270°, 5 cols × 16 filas):
//  0  L:QRT   layer activo
//  1  (vacío)
//  2  MODS    modificadores header
//  3  SCAG    valores (mayúscula=activo, punto=inactivo)
//  4  (vacío)
//  5  W:nnn   WPM actual
//  6  H:nnn   WPM máximo histórico (EEPROM)
//  7  ##+..   barra de cadencia WPM (0–WPM_MAX)
//  8  (vacío)
//  9  WORK    pomodoro estado (parpadea cada BLINK_POMO_MS si activo)
// 10  (vacío)
// 11  25:00   tiempo restante MM:SS
// 12  (vacío)
// 13  C:1/4   ciclo actual / POMO_CYCLES
// 14–15 (vacío)
static void render_status(void) {
    render_layer();
    render_mods();
    render_wpm();
    render_best_wpm();
    render_wpm_bar();
    render_pomodoro();
}

// ── Punto de entrada del driver OLED ─────────
// pomo_tick() se llama aquí (lógica de avance) antes del render.
bool oled_task_user(void) {
    pomo_tick();
    if (is_keyboard_master())
        render_status();
    else
        render_logo();
    return false;
}

#endif // OLED_ENABLE
