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
#include "binding.c"
#include "active_keys.h"
#include "report_queue.h"
#include "blocked_keys.h"
#include "matrix.h"
#include "hhstdio.h"

struct
{
  uint32_t       row_data[NUM_ROWS];
  ActiveKeys     active_keys;
  bool           error_roll_over;
  KeyMap         active_keymap;      // keymap used to determine current bindings
  KeyMap         selected_keymap;    // the 'current default'. Reset to this every cycle.
  KeyMap         default_keymap;     // used at startup and when a mode it toggled off
  KeyMap         prev_keymap;
  KeyboardReport prev_report;
} kb;

static void    reset(void);
static void    scan_matrix(void);
static void    init_active_keys(void);
static void    update_bindings(KeyboardReport *report);
static bool    momentary_mode_engaged(void);
static bool    modifier_keys_engaged(KeyboardReport *report);
static void    maybe_toggle_mode(void);
static void    process_keys(KeyboardReport *report);
static bool    modifiers_would_change(const PreMods *premods, uint8_t current_mods, uint8_t target_mods);
static uint8_t fill_report(USB_KeyboardReport_Data_t *report);
static void    toggle_map(KeyMap mode_map);
static void    stdout_to_report_queue(void);

void
Keyboard__init()
{
  stdio_init();
  init_cols();

  kb.default_keymap  = (KeyMap) pgm_read_word(&kbd_map_mx_default);
  kb.selected_keymap = kb.default_keymap;
  kb.active_keymap   = NULL;
  kb.prev_keymap     = kb.default_keymap;

  BlockedKeys__init();
  ReportQueue__init();
  reset();
}

void
stdout_to_report_queue()
{
  char ch;
  KeyboardReport *report;

  while((ReportQueue__freespace() >= 2) && (ch = stdout_popchar()))
  {
    report = ReportQueue__push();
    stdio_fill_report(ch, &report->report);

    report = ReportQueue__push();
  }
}

bool
Keyboard__key_is_down()
{
  scan_matrix();
  for (uint8_t row = 0; row < NUM_ROWS; ++row)
  {
    if (kb.row_data[row] != 0)
      return true;
  }
  return false;
}

uint8_t
Keyboard__get_report(USB_KeyboardReport_Data_t *report)
{
  reset();

  if (!stdout_is_empty())
    stdout_to_report_queue();

  if (ReportQueue__is_empty())
  {
    scan_matrix();
    init_active_keys();

    if (!kb.error_roll_over)
    {
      KeyboardReport *report = ReportQueue__push();
      do
        update_bindings(report);
      while (momentary_mode_engaged() || modifier_keys_engaged(report));
      maybe_toggle_mode();
      process_keys(report);
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
    // into a single 32 bit value.
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

#ifdef BLOCK_GHOST_KEYS
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
#endif // BLOCK_GHOST_KEYS
  }
}

void
update_bindings(KeyboardReport *report)
{
  for (BoundKey* key = ActiveKeys__first(&kb.active_keys);
       key;      key = ActiveKeys__next(&kb.active_keys))
  {
    BoundKey__update_binding(key, KeyboardReport__get_modifiers(report), kb.active_keymap);
  }
}

bool
momentary_mode_engaged()
{
  bool result = false;
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
        result = true;
        break;
      }
    }
  }

  // If the active mode is changing, all active (non-modifier)
  // keys must be deactivated and blocked.  Solves issue #20.
  if (kb.active_keymap != kb.prev_keymap)
  {
    kb.prev_keymap = kb.active_keymap;
    for (BoundKey* key = ActiveKeys__first(&kb.active_keys);
        key;       key = ActiveKeys__next(&kb.active_keys))
    {
      if (key->binding.kind == MAP)
      {
        const MapTarget *target = KeyBinding__get_map_target(&key->binding);
        if (get_modifier(target->usage) == NONE)
        {
          BlockedKeys__block_key(key->cell);
          BoundKey__deactivate(key);
        }
      }
    }
  }

  return result;
}

bool
modifier_keys_engaged(KeyboardReport *report)
{
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
        KeyboardReport__reset_modifiers(report, key->binding.premods.std);
        KeyboardReport__reset_modifiers(report, key->binding.premods.any);
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

bool
modifiers_would_change(const PreMods *premods, uint8_t current_mods, uint8_t target_mods)
{
  return (((current_mods & ~premods->std) & ~premods->any) | target_mods) != current_mods;
}

void
process_keys(KeyboardReport *report)
{
  // Binding with modifier overlap problem
  // -------------------------------------
  // Key bindings with that change modifiers conflict with
  // bindings that don't. That is, they cannot be sent in the
  // same report. To avoid the conflict, we must try to
  // determine the intended sequence of the keys, and simulate
  // that the keys were actually pressed and released in that
  // sequence.

  // To do this, we must first determine if we are in the overlap
  // problem state by checking two things:
  // * if there are two or more keys in the active set.
  // * if any of these keys would cause a modifier change.

  // Once we've determined that we're in the problem state, we try
  // to determine the sequence of keys.  If the current key was
  // active in the previous cycle, we assume that this key is on
  // its way out, so we block the key until it is released.
  // Otherwise, we assume that all of the other keys in this cycle
  // are on their way out, so we block them until they are
  // released.

  // Problem with this approach
  // --------------------------
  // This only really works under the assumption that there are
  // only two keys to deal with, and that they are the result of
  // an overlap caused by fast typing (as one key is being
  // released, another is being pressed). There is no way (short
  // of writing a custom keyboard driver) of correctly handling
  // multiple simultaneous keys when some have modifiers and some
  // don't.  This is a limitation of the HID keyboard protocol.

  bool block_others = false;

  // First, process keys with modifiers
  for (BoundKey* key = ActiveKeys__first(&kb.active_keys);
       key;      key = ActiveKeys__next(&kb.active_keys))
  {
    if (key->binding.kind == MAP)
    {
      uint8_t current_mods = KeyboardReport__get_modifiers(report);
      const MapTarget *target = KeyBinding__get_map_target(&key->binding);
      if (ActiveKeys__count(&kb.active_keys) > 1
          && modifiers_would_change(&key->binding.premods, current_mods, target->modifiers))
      {
        if (KeyboardReport__has_key(&kb.prev_report, target->usage))
        {
          // Key is leaving, block it
          BlockedKeys__block_key(key->cell);
          BoundKey__deactivate(key);
        }
        else
        {
          // Key is arriving, assume others are leaving and block them
          block_others = true;
          KeyboardReport__add_key(report, target->usage);
          KeyboardReport__reset_modifiers(report, key->binding.premods.std);
          KeyboardReport__reset_modifiers(report, key->binding.premods.any);
          KeyboardReport__set_modifiers(report, target->modifiers);
          BoundKey__deactivate(key);
        }
      }
    }
  }

  // Then, process keys without modifiers
  for (BoundKey* key = ActiveKeys__first(&kb.active_keys);
       key;      key = ActiveKeys__next(&kb.active_keys))
  {
    if (block_others)
    {
      BlockedKeys__block_key(key->cell);
    }
    else
    {
      switch (key->binding.kind)
      {
      case MAP:
        {
          const MapTarget *target = KeyBinding__get_map_target(&key->binding);
          KeyboardReport__add_key(report, target->usage);
          KeyboardReport__reset_modifiers(report, key->binding.premods.std);
          KeyboardReport__reset_modifiers(report, key->binding.premods.any);
          KeyboardReport__set_modifiers(report, target->modifiers);
          break;
        }
      case MACRO:
        {
          const MacroTarget *macro = KeyBinding__get_macro_target(&key->binding);
          BlockedKeys__block_key(key->cell);
          ReportQueue__pop();  // current report no longer needed.

          for (int i = 0; i < macro->length; ++i)
          {
            if (ReportQueue__freespace() < 2)
              break;  // TODO: ensure macro size < queue capacity

            report = ReportQueue__push();
            const MapTarget *target = MacroTarget__get_map_target(macro, i);
            KeyboardReport__add_key(report, target->usage);
            KeyboardReport__set_modifiers(report, target->modifiers);

            // add a blank report to simulate key-up.  We need to do this
            // otherwise sequences of identical keys will be treated as
            // one by the host.
            report = ReportQueue__push();
            KeyboardReport__set_modifiers(report, target->modifiers);
          }
          break;
        }
      default:
        break;
      }
    }
  }
}

uint8_t
fill_report(USB_KeyboardReport_Data_t *dest_report)
{
  memset(dest_report, 0, sizeof(*dest_report));

  KeyboardReport *report = ReportQueue__pop();
  if (report)
  {
    if (!kb.error_roll_over)
    {
      KeyboardReport__copy(report, dest_report);
      KeyboardReport__init_copy(report, &kb.prev_report);
    }
    else
    {
      KeyboardReport__init_error(report, dest_report);
      KeyboardReport__init_error_copy(report, &kb.prev_report);
    }
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


