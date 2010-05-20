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

#include "Keyboard.h"
#include "keyboard_state.h"
#include <string.h>
#include <limits.h>

KeyboardState g_kb_state;

void
keyboard_state__reset()
{
  g_kb_state.num_active_cells = g_kb_state.num_keys = 0;
  g_kb_state.error_roll_over = false;
  memset(&g_kb_state.keys[0], UCHAR_MAX, sizeof(g_kb_state.keys[0])*MAX_KEYS);
  memset(&g_kb_state.active_cells[0], UCHAR_MAX, sizeof(g_kb_state.active_cells[0])*MAX_ACTIVE_CELLS);
}

bool
keyboard_state__is_error()
{
  return g_kb_state.error_roll_over;
}

bool
keyboard_state__is_empty()
{
  return g_kb_state.num_active_cells == 0;
}

void
keyboard_state__get_active_cells()
{
  uint8_t ncols;
  // now process row/column data to get raw keypresses
  for (uint8_t row = 0; row < NUM_ROWS; ++row)
  {
    ncols = 0;
    for (uint8_t col = 0; col < NUM_COLS; ++col)
    {
      if (g_kb_state.row_data[row] & (1UL << col))
      {
        if (g_kb_state.num_active_cells > MAX_ACTIVE_CELLS)
        {
          g_kb_state.error_roll_over = true;
          return;
        }
        ++ncols;
        g_kb_state.active_cells[g_kb_state.num_active_cells++] = MATRIX_CELL(row, col);
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
        if (g_kb_state.row_data[row] & g_kb_state.row_data[irow])
        {
          g_kb_state.error_roll_over = true;
          return;
        }
      }
    }
  }
}

uint8_t
keyboard_state__fill_report(USB_KeyboardReport_Data_t *report)
{
  if (keyboard_state__is_empty())
    return 0;

  if (keyboard_state__is_error())
  {
    report->Modifier = g_kb_state.modifiers;
    for (uint8_t key = 1; key < 7; ++key)
      report->KeyCode[key] = USAGE_ID(HID_USAGE_ERRORROLLOVER);
  }
  else if (!keyboard_state__is_processing_macro())
  {
    report->Modifier = g_kb_state.modifiers;
    for (uint8_t key = 0; key < g_kb_state.num_keys; ++key)
      report->KeyCode[key] = g_kb_state.keys[key];
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
keyboard_state__is_processing_macro()
{
  return false;
#if 0 // FIXME
  return g_kb_state.macro != NULL;
#endif
}


