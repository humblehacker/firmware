/*
                    The HumbleHacker Keyboard Project
                 Copyright © 2008-2010, David Whetstone
               david DOT whetstone AT humblehacker DOT com

  This file is a part of The HumbleHacker Keyboard Project.

  The HumbleHacker Keyboard Project is free software: you can redistribute
  it and/or modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  The HumbleHacker Keyboard Project is distributed in the hope that it will
  be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
  Public License for more details.

  You should have received a copy of the GNU General Public License along
  with The HumbleHacker Keyboard Project.  If not, see
  <http://www.gnu.org/licenses/>.

*/

#include <string.h>
#include <limits.h>
#include <util/delay.h>

#include "Keyboard.h"
#include "keyboard_class.h"
#include "keymaps.h"
#include "conf_keyboard.h"
#include "binding.c"

static Modifiers get_modifier(Usage usage);

#define NUM_MODIFIERS 8
#define MAX_KEYS      6
#define MAX_ACTIVE_CELLS (MAX_KEYS + NUM_MODIFIERS)
//#define MAX_ACTIVE_CELLS (MAX_KEYS + NUM_MODE_KEYS + NUM_MODIFIERS)

typedef struct
{
  uint32_t  row_data[NUM_ROWS];
  Modifiers modifiers;
  Usage     keys[MAX_KEYS];
  uint8_t   num_keys;
  BoundKey  active_keys[MAX_ACTIVE_CELLS];
  uint8_t   num_active_keys;
  uint8_t   curr_active_key;
  const     MacroTarget *macro;
  bool      error_roll_over;
  KeyMap    active_keymap;
  KeyMap    current_keymap;
  KeyMap    default_keymap;
} Keyboard;

Keyboard keyboard;

void
Keyboard__init()
{
  init_cols();

  keyboard.default_keymap = (KeyMap) pgm_read_word(&kbd_map_mx_default);
  keyboard.current_keymap = keyboard.default_keymap;
  keyboard.active_keymap = NULL;

  Keyboard__reset();
}

void
Keyboard__reset()
{
  memset(&keyboard.active_keys[0], UCHAR_MAX, sizeof(keyboard.active_keys[0])*MAX_ACTIVE_CELLS);
  keyboard.num_active_keys = 0;

  memset(&keyboard.keys[0], UCHAR_MAX, sizeof(keyboard.keys[0])*MAX_KEYS);
  keyboard.num_keys = 0;

  keyboard.error_roll_over = false;
  keyboard.modifiers = NONE;

  keyboard.active_keymap = keyboard.current_keymap;
}

bool
Keyboard__is_error()
{
  return keyboard.error_roll_over;
}

void
Keyboard__scan_matrix()
{
  init_cols();
  for (uint8_t row = 0; row < NUM_ROWS; ++row)
  {
    activate_row(row);

    // Insert NOPs for synchronization
    _delay_us(20);

    // Place data on all column pins for active row into a single
    // 32 bit value.
		keyboard.row_data[row] = 0;
    keyboard.row_data[row] = read_row_data();
  }
}

void
Keyboard__update_bindings(void)
{
  for (BoundKey* key = Keyboard__first_active_key();
       key;      key = Keyboard__next_active_key())
  {
    BoundKey__update_binding(key, keyboard.modifiers, keyboard.active_keymap);
  }
}


void
Keyboard__init_active_keys()
{
  BoundKey *key = NULL;
  uint8_t ncols;
  // now process row/column data to get raw keypresses
  for (uint8_t row = 0; row < NUM_ROWS; ++row)
  {
    ncols = 0;
    for (uint8_t col = 0; col < NUM_COLS; ++col)
    {
      if (keyboard.row_data[row] & (1UL << col))
      {
        if (keyboard.num_active_keys > MAX_ACTIVE_CELLS)
        {
          keyboard.error_roll_over = true;
          return;
        }
        ++ncols;
        key = &keyboard.active_keys[keyboard.num_active_keys++];
        key->cell = MATRIX_CELL(row, col);
        key->binding = NULL;
      }
    }

    // if 2 or more keys pressed in a row, check for ghost-key
    if (ncols > 1)
    {
      for (uint8_t irow = 0; irow < NUM_ROWS; ++irow)
      {
        if (irow == row)
          continue;

        // if any other row has a key pressed in the same column as any
        // of the two or more keys pressed in the current row, we have a
        // ghost-key condition.
        if (keyboard.row_data[row] & keyboard.row_data[irow])
        {
          keyboard.error_roll_over = true;
          return;
        }
      }
    }
  }
}

bool
Keyboard__momentary_mode_engaged()
{
  for (BoundKey* key = Keyboard__first_active_key();
       key;      key = Keyboard__next_active_key())
  {
    if (key->binding->kind == MODE)
    {
      ModeTarget *target = (ModeTarget*)key->binding->target;
      if (target->type == MOMENTARY)
      {
        keyboard.active_keymap = target->mode_map;
        BoundKey__deactivate(key);
        return true;
      }
    }
  }
  return false;
}

bool
Keyboard__modifier_keys_engaged()
{
  Modifiers active_modifiers = NONE;
  for (BoundKey* key = Keyboard__first_active_key();
       key;      key = Keyboard__next_active_key())
  {
    if (key->binding->kind == MAP)
    {
      const MapTarget *target = (const MapTarget*)key->binding->target;
      Modifiers this_modifier = NONE;
      if ((this_modifier = get_modifier(target->usage)) != NONE)
      {
        active_modifiers |= this_modifier;
        BoundKey__deactivate(key);
      }
    }
  }
  keyboard.modifiers |= active_modifiers;
  return active_modifiers != NONE;
}

void
Keyboard__toggle_map(KeyMap mode_map)
{
  if (keyboard.current_keymap == mode_map)
    keyboard.current_keymap = keyboard.default_keymap;
  else
    keyboard.current_keymap = mode_map;
}

void
Keyboard__check_mode_toggle(void)
{
  for (BoundKey* key = Keyboard__first_active_key();
       key;      key = Keyboard__next_active_key())
  {
    if (key->binding->kind == MODE)
    {
      ModeTarget *target = (ModeTarget*)key->binding->target;
      if (target->type == TOGGLE)
      {
        Keyboard__toggle_map(target->mode_map);
        BoundKey__deactivate(key);
        return;
      }
    }
  }
}

void
Keyboard__process_keys()
{
  for (BoundKey* key = Keyboard__first_active_key();
       key;      key = Keyboard__next_active_key())
  {
    if (key->binding->kind == MAP)
    {
      const MapTarget *target = (const MapTarget*)key->binding->target;
      keyboard.keys[keyboard.num_keys] = target->usage;
      keyboard.modifiers &= ~key->binding->premods;
      keyboard.modifiers |= target->modifiers;
      ++keyboard.num_keys;
    }
  }
}

uint8_t
Keyboard__fill_report(USB_KeyboardReport_Data_t *report)
{
  if (Keyboard__is_error())
  {
    report->Modifier = keyboard.modifiers;
    for (uint8_t key = 1; key < 7; ++key)
      report->KeyCode[key] = USAGE_ID(HID_USAGE_ERRORROLLOVER);
  }
  else if (!Keyboard__is_processing_macro())
  {
    report->Modifier = keyboard.modifiers;
    for (uint8_t key = 0; key < keyboard.num_keys; ++key)
      report->KeyCode[key] = keyboard.keys[key];
  }
  else
  {
    // TODO: Macro processing
#if 0
    const Macro * macro = g_kb_state.macro;
    MacroKey mkey;
    mkey.mod.all = pgm_read_byte(&macro->keys[g_kb_state.macro_key_index].mod);
    mkey.usage = pgm_read_word(&macro->keys[g_kb_state.macro_key_index].usage);
    uint8_t num_macro_keys = pgm_read_byte(&macro->num_keys);
    report->Modifier = g_kb_state.pre_macro_modifiers | mkey.mod.all;
    report->KeyCode[0] = USAGE_ID(mkey.usage);
    g_kb_state.macro_key_index++;
    if (g_kb_state.macro_key_index >= num_macro_keys)
    {
      g_kb_state.macro = NULL;
      g_kb_state.macro_key_index = 0;
    }
#endif
  }
  return sizeof(USB_KeyboardReport_Data_t);
}

bool
Keyboard__is_processing_macro()
{
  return false;
#if 0 // FIXME
  return g_kb_state.macro != NULL;
#endif
}

BoundKey*
Keyboard__first_active_key(void)
{
  keyboard.curr_active_key = 0;
  BoundKey *key = &keyboard.active_keys[keyboard.curr_active_key];
  if (key && BoundKey__is_active(key)) return key;
  return Keyboard__next_active_key();
}

BoundKey*
Keyboard__next_active_key(void)
{
  BoundKey *key = NULL;
  while (!key && keyboard.curr_active_key < keyboard.num_active_keys)
  {
    key = &keyboard.active_keys[++keyboard.curr_active_key];
    if (!BoundKey__is_active(key))
      key = NULL;
  }
  return key;
}

static
Modifiers
get_modifier(Usage usage)
{
  switch(usage)
  {
  case HID_USAGE_LEFT_CONTROL:
    return L_CTL;
  case HID_USAGE_LEFT_SHIFT:
    return L_SHF;
  case HID_USAGE_LEFT_ALT:
    return L_ALT;
  case HID_USAGE_LEFT_GUI:
    return L_GUI;
  case HID_USAGE_RIGHT_CONTROL:
    return R_CTL;
  case HID_USAGE_RIGHT_SHIFT:
    return R_SHF;
  case HID_USAGE_RIGHT_ALT:
    return R_ALT;
  case HID_USAGE_RIGHT_GUI:
    return R_GUI;
  default:
    return NONE;
  }
}

