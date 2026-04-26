/*
This is the c configuration file for the keymap

Copyright 2012 Jun Wako <wakojun@gmail.com>
Copyright 2015 Jack Humbert

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

/* VIAL UID for Lily58*/
#define VIAL_KEYBOARD_UID {0x7E, 0xFD, 0xFC, 0x5B, 0x7D, 0x39, 0x48, 0x06}

/* VIAL secure unlock keystroke - currently both big keys (typ. SPACE/ENTER) */
#define VIAL_UNLOCK_COMBO_ROWS {4, 9}
#define VIAL_UNLOCK_COMBO_COLS {4, 4}

/* Space reduction */

#define DYNAMIC_KEYMAP_LAYER_COUNT 5
#define VIAL_TAP_DANCE_ENTRIES 4
#undef LOCKING_SUPPORT_ENABLE
#undef LOCKING_RESYNC_ENABLE
#define NO_ACTION_ONESHOT

/* Select hand configuration
 * MASTER_LEFT: USB is always plugged into the left half.
 * Master detection uses hardware VBUS (~5µs), no EEPROM dependency,
 * no USB enumeration timeout — eliminates the "dead keyboard on boot" bug. */
#define MASTER_LEFT
#define USE_SERIAL_PD2

/* Tapping — home row mods
 * TAPPING_TERM: time window (ms) to distinguish tap from hold.
 * PERMISSIVE_HOLD: activates hold when another key is pressed before timeout.
 * CHORDAL_HOLD: only treat a mod-tap as held when the next key is on the
 *   opposite hand (or in the thumb cluster, marked '*' in chordal_hold_layout).
 *   This prevents accidental modifier activation during same-hand rolls.
 * IGNORE_MOD_TAP_INTERRUPT: prevents accidental hold during fast typing. */
#define TAPPING_TERM 300
#define PERMISSIVE_HOLD
#define CHORDAL_HOLD
// IGNORE_MOD_TAP_INTERRUPT is now the default behavior in QMK — no longer needed

/* OLED — 30fps refresh (33ms per frame)
 * QMK default for split keyboards is 50ms (20fps).
 * 33ms is the practical maximum on ATmega32u4 at 400kHz I2C
 * without degrading key scan or split communication. */
#define OLED_UPDATE_INTERVAL 33
