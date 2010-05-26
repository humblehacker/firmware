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
#include "active_keys.h"

static Modifiers get_modifier(Usage usage);

#define NUM_MODIFIERS 8
#define MAX_KEYS      6
#define MAX_ACTIVE_CELLS (MAX_KEYS + NUM_MODIFIERS)
//#define MAX_ACTIVE_CELLS (MAX_KEYS + NUM_MODE_KEYS + NUM_MODIFIERS)

typedef struct
{
  uint32_t                  row_data[NUM_ROWS];
  uint8_t                   num_keys;
  ActiveKeys                active_keys;
  const MacroTarget        *macro;
  bool                      error_roll_over;
  KeyMap                    active_keymap;
  KeyMap                    selected_keymap;
  KeyMap                    default_keymap;
  USB_KeyboardReport_Data_t report;
} Keyboard;

Keyboard keyboard;

static void      Keyboard__reset(void);

static void      Keyboard__scan_matrix(void);

static bool      Keyboard__is_error(void);
static uint8_t   Keyboard__fill_report(USB_KeyboardReport_Data_t *report);
static bool      Keyboard__is_processing_macro(void);

static bool      Keyboard__momentary_mode_engaged(void);
static bool      Keyboard__modifier_keys_engaged(void);
static void      Keyboard__maybe_toggle_mode(void);
static void      Keyboard__process_keys(void);
static void      Keyboard__toggle_map(KeyMap mode_map);

static void      Keyboard__update_bindings(void);
static void      Keyboard__init_active_keys(void);
static BoundKey* Keyboard__first_active_key(void);
static BoundKey* Keyboard__next_active_key(void);

void
Keyboard__init()
{
  init_cols();

  keyboard.default_keymap = (KeyMap) pgm_read_word(&kbd_map_mx_default);
  keyboard.selected_keymap = keyboard.default_keymap;
  keyboard.active_keymap = NULL;

  Keyboard__reset();
}

void
Keyboard__reset()
{
  ActiveKeys__reset(&keyboard.active_keys);

  keyboard.num_keys = 0;


  memset(&keyboard.report, 0, sizeof(keyboard.report));

  keyboard.error_roll_over = false;

  keyboard.active_keymap = keyboard.selected_keymap;
}

bool
Keyboard__is_error()
{
  return keyboard.error_roll_over;
}

uint8_t
Keyboard__get_report(USB_KeyboardReport_Data_t *report)
{
  Keyboard__reset();
  Keyboard__scan_matrix();
	Keyboard__init_active_keys();

  if (!keyboard.error_roll_over)
  {
    do
      keyboard_update_bindings();
    while (keyboard.momentary_mode_engaged()
        || keyboard.modifier_keys_engaged());
    Keyboard__maybe_toggle_mode();
    Keyboard__process_keys();
  }

  return Keyboard__fill_report(report);
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
  for (BoundKey* key = ActiveKeys__first(&keyboard.active_keys);
       key;      key = ActiveKeys__next(&keyboard.active_keys))
  {
    BoundKey__update_binding(key, keyboard.report.Modifier, keyboard.active_keymap);
  }
}


void
Keyboard__init_active_keys()
{
  uint8_t ncols;
  // now process row/column data to get raw keypresses
  for (uint8_t row = 0; row < NUM_ROWS; ++row)
  {
    ncols = 0;
    for (uint8_t col = 0; col < NUM_COLS; ++col)
    {
      if (keyboard.row_data[row] & (1UL << col))
      {
        if (!ActiveKeys__add_cell(&keyboard.active_keys, MATRIX_CELL(row, col)))
        {
          keyboard.error_roll_over = true;
          return;
        }
        ++ncols;
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
  for (BoundKey* key = ActiveKeys__first(&keyboard.active_keys);
       key;      key = ActiveKeys__next(&keyboard.active_keys))
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
  for (BoundKey* key = ActiveKeys__first(&keyboard.active_keys);
       key;      key = ActiveKeys__next(&keyboard.active_keys))
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
  keyboard.report.Modifier |= active_modifiers;
  return active_modifiers != NONE;
}

void
Keyboard__toggle_map(KeyMap mode_map)
{
  if (keyboard.selected_keymap == mode_map)
    keyboard.selected_keymap = keyboard.default_keymap;
  else
    keyboard.selected_keymap = mode_map;
}

void
Keyboard__maybe_toggle_mode(void)
{
  for (BoundKey* key = ActiveKeys__first(&keyboard.active_keys);
       key;      key = ActiveKeys__next(&keyboard.active_keys))
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
  for (BoundKey* key = ActiveKeys__first(&keyboard.active_keys);
       key;      key = ActiveKeys__next(&keyboard.active_keys))
  {
    if (key->binding->kind == MAP)
    {
      const MapTarget *target = (const MapTarget*)key->binding->target;
      keyboard.report.KeyCode[keyboard.num_keys] = target->usage;
      keyboard.report.Modifier &= ~key->binding->premods;
      keyboard.report.Modifier |= target->modifiers;
      ++keyboard.num_keys;
    }
  }
}

uint8_t
Keyboard__fill_report(USB_KeyboardReport_Data_t *report)
{
  if (Keyboard__is_error())
  {
    report->Modifier = keyboard.report.Modifier;
    for (uint8_t key = 1; key < 7; ++key)
      report->KeyCode[key] = USAGE_ID(HID_USAGE_ERRORROLLOVER);
  }
  else if (!Keyboard__is_processing_macro())
  {
    memcpy(report, &keyboard.report, sizeof(keyboard.report));
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

