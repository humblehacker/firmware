/*
                 The HumbleHacker Keyboard Project
                 Copyright © 2008, David Whetstone
            david DOT whetstone AT humblehacker DOT com

  This file is a part of The HumbleHacker Keyboard Project.

  The HumbleHacker Keyboard Project is free software: you can
  redistribute it and/or modify it under the terms of the GNU General
  Public License as published by the Free Software Foundation, either
  version 3 of the License, or (at your option) any later version.

  The HumbleHacker Keyboard Project is distributed in the hope that it
  will be useful, but WITHOUT ANY WARRANTY; without even the implied
  warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with The HumbleHacker Keyboard Project.  If not, see
  <http://www.gnu.org/licenses/>.

*/

#include "Keyboard.h"
#include "keyboard_state.h"
#include <string.h>
#include <limits.h>

KeyboardState g_kb_state;
uint8_t g_num_blocked_keys;
uint8_t g_blocked_keys[MAX_ACTIVE_CELLS];

void
keyboard_state__reset()
{
  g_kb_state.num_active_cells = g_kb_state.num_keys = 0;
  g_kb_state.error_roll_over = false;
  memset(&g_kb_state.keys[0], UCHAR_MAX, sizeof(g_kb_state.keys[0])*MAX_KEYS);
  memset(&g_kb_state.active_cells[0], UCHAR_MAX, sizeof(g_kb_state.active_cells[0])*MAX_ACTIVE_CELLS);
}

uint8_t
keyboard_state__is_error()
{
  return g_kb_state.error_roll_over;
}

uint8_t
keyboard_state__is_empty()
{
  return g_kb_state.num_active_cells == 0;
}

uint8_t
keyboard_state__is_processing_macro()
{
  return false;
#if 0 // FIXME
  return g_kb_state.macro != NULL;
#endif
}


