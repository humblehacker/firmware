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
#include "report_queue.h"
#include "blocked_keys.h"

struct
{
  uint32_t    row_data[NUM_ROWS];
  ActiveKeys  active_keys;
  bool        error_roll_over;
  KeyMap      active_keymap;      // keymap used to determine current bindings
  KeyMap      selected_keymap;    // the 'current default'. Reset to this every cycle.
  KeyMap      default_keymap;     // used at startup and when a mode it toggled off
} kb;

static void    reset(void);
static void    scan_matrix(void);
static void    init_active_keys(void);
static void    update_bindings(void);
static bool    momentary_mode_engaged(void);
static bool    modifier_keys_engaged(void);
static void    maybe_toggle_mode(void);
static void    process_keys(void);
static uint8_t fill_report(USB_KeyboardReport_Data_t *report);
static void    toggle_map(KeyMap mode_map);

void
Keyboard__init()
{
  init_cols();

  kb.default_keymap  = (KeyMap) pgm_read_word(&kbd_map_mx_default);
  kb.selected_keymap = kb.default_keymap;
  kb.active_keymap   = NULL;

  BlockedKeys__init();
  ReportQueue__init();
  reset();
}

uint8_t
Keyboard__get_report(USB_KeyboardReport_Data_t *report)
{
  reset();
  if (ReportQueue__is_empty())
  {
    scan_matrix();
    init_active_keys();

    if (!kb.error_roll_over)
    {
      ReportQueue__push();
      do
        update_bindings();
      while (momentary_mode_engaged() || modifier_keys_engaged());
      maybe_toggle_mode();
      process_keys();
    }
  }

  return fill_report(report);
}

void
reset()
{
  ActiveKeys__reset(&kb.active_keys);

  kb.error_roll_over = false;
  kb.active_keymap   = kb.selected_keymap;
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
init_active_keys()
{
  uint8_t ncols, cell;
  // process row/column data to find the active keys
  for (uint8_t row = 0; row < NUM_ROWS; ++row)
  {
    ncols = 0;
    for (uint8_t col = 0; col < NUM_COLS; ++col)
    {
      cell = MATRIX_CELL(row, col);
      if (kb.row_data[row] & (1UL << col))
      {
        if (!BlockedKeys__is_blocked(cell))
        {
          if (!ActiveKeys__add_cell(&kb.active_keys, cell))
          {
            kb.error_roll_over = true;
            return;
          }
        }
        ++ncols;
      }
      else
      {
        BlockedKeys__unblock_key(cell);
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

void
update_bindings()
{
  KeyboardReport *report = ReportQueue__peek();
  for (BoundKey* key = ActiveKeys__first(&kb.active_keys);
       key;      key = ActiveKeys__next(&kb.active_keys))
  {
    BoundKey__update_binding(key, KeyboardReport__get_modifiers(report), kb.active_keymap);
  }
}

bool
momentary_mode_engaged()
{
  for (BoundKey* key = ActiveKeys__first(&kb.active_keys);
       key;      key = ActiveKeys__next(&kb.active_keys))
  {
    if (key->binding.kind == MODE)
    {
      const ModeTarget *target = KeyBinding__get_mode_target(&key->binding);
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
  KeyboardReport *report = ReportQueue__peek();
  Modifiers active_modifiers = NONE;
  for (BoundKey* key = ActiveKeys__first(&kb.active_keys);
       key;      key = ActiveKeys__next(&kb.active_keys))
  {
    if (key->binding.kind == MAP)
    {
      const MapTarget *target = KeyBinding__get_map_target(&key->binding);
      Modifiers this_modifier = NONE;
      if ((this_modifier = get_modifier(target->usage)) != NONE)
      {
        active_modifiers |= this_modifier;
        KeyboardReport__reset_modifiers(report, key->binding.premods);
        BoundKey__deactivate(key);
      }
    }
  }
  KeyboardReport__set_modifiers(report, active_modifiers);
  return active_modifiers != NONE;
}

void
maybe_toggle_mode(void)
{
  for (BoundKey* key = ActiveKeys__first(&kb.active_keys);
       key;      key = ActiveKeys__next(&kb.active_keys))
  {
    if (key->binding.kind == MODE)
    {
      const ModeTarget *target = KeyBinding__get_mode_target(&key->binding);
      if (target->type == TOGGLE)
      {
        toggle_map(target->mode_map);
        BoundKey__deactivate(key);
        BlockedKeys__block_key(key->cell);
        return;
      }
    }
  }
}

void
process_keys()
{
  KeyboardReport *report      = ReportQueue__peek();
  KeyboardReport *prev_report = ReportQueue__prev();

  bool block_others = false;

  // First, process keys with modifiers
  for (BoundKey* key = ActiveKeys__first(&kb.active_keys);
       key;      key = ActiveKeys__next(&kb.active_keys))
  {
    if (key->binding.kind == MAP)
    {
      const MapTarget *target = KeyBinding__get_map_target(&key->binding);
      if (target->modifiers != 0 &&
          ActiveKeys__count(&kb.active_keys) > 1 &&
          !(KeyboardReport__get_modifiers(report) & target->modifiers))
      {
        if (KeyboardReport__has_key(prev_report, target->usage))
        {
          BlockedKeys__block_key(key->cell);
        }
        else
        {
          block_others = true;
          KeyboardReport__add_key(report, target->usage);
          KeyboardReport__reset_modifiers(report, key->binding.premods);
          KeyboardReport__set_modifiers(report, target->modifiers);
        }
        BoundKey__deactivate(key);
      }
    }
  }

  // Then, process keys without modifiers
  for (BoundKey* key = ActiveKeys__first(&kb.active_keys);
       key;      key = ActiveKeys__next(&kb.active_keys))
  {
    switch (key->binding.kind)
    {
    case MAP:
      {
        if (block_others)
        {
          BlockedKeys__block_key(key->cell);
        }
        else
        {
          const MapTarget *target = KeyBinding__get_map_target(&key->binding);
          KeyboardReport__add_key(report, target->usage);
          KeyboardReport__reset_modifiers(report, key->binding.premods);
          KeyboardReport__set_modifiers(report, target->modifiers);
        }
        break;
      }
    case MACRO:
      {
        const MacroTarget *macro = KeyBinding__get_macro_target(&key->binding);
        KeyboardReport *report = NULL;
        for (int i = 0; i < macro->length; ++i)
        {
          report = ReportQueue__push();
          if (!report)  // TODO: ensure macro size < queue capacity
            break;
          const MapTarget *target = MacroTarget__get_map_target(macro, i);
          KeyboardReport__add_key(report, target->usage);
          KeyboardReport__set_modifiers(report, target->modifiers);

          // add a blank report to simulate key-up
          report = ReportQueue__push();
          if (!report)
            break;
          KeyboardReport__set_modifiers(report, target->modifiers);
        }
        break;
      }
    default:
      break;
    }
  }
}

uint8_t
fill_report(USB_KeyboardReport_Data_t *dest_report)
{
  memset(dest_report, 0, sizeof(*dest_report));

  KeyboardReport *report = ReportQueue__pop();

  if (!kb.error_roll_over)
  {
    KeyboardReport__copy(report, dest_report);
  }
  else
  {
    dest_report->Modifier = KeyboardReport__get_modifiers(report);
    for (uint8_t key = 1; key < 7; ++key)
      dest_report->KeyCode[key] = USAGE_ID(HID_USAGE_ERRORROLLOVER);
  }
  return sizeof(USB_KeyboardReport_Data_t);
}

void
toggle_map(KeyMap mode_map)
{
  if (kb.selected_keymap == mode_map)
    kb.selected_keymap = kb.default_keymap;
  else
    kb.selected_keymap = mode_map;
}


