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

struct
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
} kb;

static void    reset(void);

static void    scan_matrix(void);

static uint8_t fill_report(USB_KeyboardReport_Data_t *report);
static bool    is_processing_macro(void);

static bool    momentary_mode_engaged(void);
static bool    modifier_keys_engaged(void);
static void    maybe_toggle_mode(void);
static void    process_keys(void);
static void    toggle_map(KeyMap mode_map);

static void    update_bindings(void);
static void    init_active_keys(void);

void
Keyboard__init()
{
  init_cols();

  kb.default_keymap  = (KeyMap) pgm_read_word(&kbd_map_mx_default);
  kb.selected_keymap = kb.default_keymap;
  kb.active_keymap   = NULL;

  reset();
}

void
reset()
{
  ActiveKeys__reset(&kb.active_keys);
  memset(&kb.report, 0, sizeof(kb.report));

  kb.num_keys        = 0;
  kb.error_roll_over = false;
  kb.active_keymap   = kb.selected_keymap;
}

uint8_t
Keyboard__get_report(USB_KeyboardReport_Data_t *report)
{
  reset();
  scan_matrix();
	init_active_keys();

  if (!kb.error_roll_over)
  {
    do
      update_bindings();
    while (momentary_mode_engaged() || modifier_keys_engaged());
    maybe_toggle_mode();
    process_keys();
  }

  return fill_report(report);
}

void
scan_matrix()
{
  for (uint8_t row = 0; row < NUM_ROWS; ++row)
  {
    activate_row(row);

    // Insert NOPs for synchronization
    _delay_us(20);

    // Place data on all column pins for active row
    // into a single  32 bit value.
    kb.row_data[row] = read_row_data();
  }
}

void
update_bindings(void)
{
  for (BoundKey* key = ActiveKeys__first(&kb.active_keys);
       key;      key = ActiveKeys__next(&kb.active_keys))
  {
    BoundKey__update_binding(key, kb.report.Modifier, kb.active_keymap);
  }
}


void
init_active_keys()
{
  uint8_t ncols;
  // now process row/column data to get raw keypresses
  for (uint8_t row = 0; row < NUM_ROWS; ++row)
  {
    ncols = 0;
    for (uint8_t col = 0; col < NUM_COLS; ++col)
    {
      if (kb.row_data[row] & (1UL << col))
      {
        if (!ActiveKeys__add_cell(&kb.active_keys, MATRIX_CELL(row, col)))
        {
          kb.error_roll_over = true;
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
        if (kb.row_data[row] & kb.row_data[irow])
        {
          kb.error_roll_over = true;
          return;
        }
      }
    }
  }
}

bool
momentary_mode_engaged()
{
  for (BoundKey* key = ActiveKeys__first(&kb.active_keys);
       key;      key = ActiveKeys__next(&kb.active_keys))
  {
    if (key->binding->kind == MODE)
    {
      ModeTarget *target = (ModeTarget*)key->binding->target;
      if (target->type == MOMENTARY)
      {
        kb.active_keymap = target->mode_map;
        BoundKey__deactivate(key);
        return true;
      }
    }
  }
  return false;
}

bool
modifier_keys_engaged()
{
  Modifiers active_modifiers = NONE;
  for (BoundKey* key = ActiveKeys__first(&kb.active_keys);
       key;      key = ActiveKeys__next(&kb.active_keys))
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
  kb.report.Modifier |= active_modifiers;
  return active_modifiers != NONE;
}

void
toggle_map(KeyMap mode_map)
{
  if (kb.selected_keymap == mode_map)
    kb.selected_keymap = kb.default_keymap;
  else
    kb.selected_keymap = mode_map;
}

void
maybe_toggle_mode(void)
{
  for (BoundKey* key = ActiveKeys__first(&kb.active_keys);
       key;      key = ActiveKeys__next(&kb.active_keys))
  {
    if (key->binding->kind == MODE)
    {
      ModeTarget *target = (ModeTarget*)key->binding->target;
      if (target->type == TOGGLE)
      {
        toggle_map(target->mode_map);
        BoundKey__deactivate(key);
        return;
      }
    }
  }
}

void
process_keys()
{
  for (BoundKey* key = ActiveKeys__first(&kb.active_keys);
       key;      key = ActiveKeys__next(&kb.active_keys))
  {
    if (key->binding->kind == MAP)
    {
      const MapTarget *target = (const MapTarget*)key->binding->target;
      kb.report.KeyCode[kb.num_keys] = target->usage;
      kb.report.Modifier &= ~key->binding->premods;
      kb.report.Modifier |= target->modifiers;
      ++kb.num_keys;
    }
  }
}

uint8_t
fill_report(USB_KeyboardReport_Data_t *report)
{
  if (kb.error_roll_over)
  {
    report->Modifier = kb.report.Modifier;
    for (uint8_t key = 1; key < 7; ++key)
      report->KeyCode[key] = USAGE_ID(HID_USAGE_ERRORROLLOVER);
  }
  else if (!is_processing_macro())
  {
    memcpy(report, &kb.report, sizeof(kb.report));
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
is_processing_macro()
{
  return false;
#if 0 // FIXME
  return g_kb_state.macro != NULL;
#endif
}

