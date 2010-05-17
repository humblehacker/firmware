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

#include "config.h"
#include "keyboard_state.h"
#include <string.h>
#include <limits.h>

static KeyboardState kb_state_1, kb_state_2;
static KeyboardState *prev_kb_state;
KeyboardState *g_current_kb_state;
uint8_t g_num_blocked_keys;
uint8_t g_blocked_keys[MAX_ACTIVE_CELLS];

static
void
init_keyboard_state(KeyboardState *kb_state)
{
  kb_state->modifiers.all = kb_state->mode_keys = 0;
  kb_state->num_active_cells = kb_state->num_keys = 0;
//kb_state->macro = NULL;
  kb_state->macro_key_index = 0;
  kb_state->pre_macro_modifiers = 0;
  kb_state->error_roll_over = FALSE;
  kb_state->consumer_key = 0;
  memset(&kb_state->keys[0], UCHAR_MAX, sizeof(kb_state->keys[0])*MAX_KEYS);
  memset(&kb_state->active_cells[0], UCHAR_MAX, sizeof(kb_state->active_cells[0])*MAX_ACTIVE_CELLS);
}

void
keyboard_state__init()
{
  init_keyboard_state(&kb_state_1);
  init_keyboard_state(&kb_state_2);
  g_current_kb_state = &kb_state_1;
  prev_kb_state      = &kb_state_2;
}

void
keyboard_state__reset_current_state()
{
  init_keyboard_state(g_current_kb_state);
}

uint8_t
keyboard_state__is_error()
{
  return g_current_kb_state->error_roll_over;
}

uint8_t
keyboard_state__is_empty()
{
  return g_current_kb_state->num_active_cells == 0;
}

uint8_t
keyboard_state__is_processing_macro()
{
  return false;
#if 0 // FIXME
  return g_current_kb_state->macro != NULL;
#endif
}

uint8_t
keyboard_state__has_changed()
{
  if (kb_state_1.num_active_cells != kb_state_2.num_active_cells)
    return TRUE;

  if (kb_state_1.error_roll_over != kb_state_2.error_roll_over)
    return TRUE;

  if (kb_state_1.macro || kb_state_2.macro)
    return TRUE;

  uint8_t i;
  for (i=0; i< kb_state_1.num_active_cells; ++i)
  {
    if (kb_state_1.active_cells[i] != kb_state_2.active_cells[i])
      return TRUE;
  }

  return FALSE;
}

uint8_t
keyboard_state__cooked_keys_have_changed()
{
  if (kb_state_1.num_keys != kb_state_2.num_keys)
    return TRUE;

  if (kb_state_1.modifiers.all != kb_state_2.modifiers.all)
    return TRUE;

  uint8_t i;
  for (i=0; i< kb_state_1.num_keys; ++i)
  {
    if (kb_state_1.keys[i] != kb_state_2.keys[i])
      return TRUE;
  }

  return FALSE;
}

uint8_t
keyboard_state__mode_keys_have_changed()
{
  return kb_state_1.mode_keys != kb_state_2.mode_keys;
}

void
keyboard_state__swap_states()
{
  KeyboardState *tmp = prev_kb_state;
  prev_kb_state = g_current_kb_state;
  g_current_kb_state = tmp;
  g_current_kb_state->modifiers.all = 0;
}

uint8_t
keyboard_state__key_is_blocked(uint8_t key)
{
  uint8_t i;
  for(i = 0; i < g_num_blocked_keys; ++i)
  {
    if(g_blocked_keys[i] == key)
      return TRUE;
  }
  return FALSE;
}

uint8_t
keyboard_state__should_send_empty_consumer_key()
{
  return g_current_kb_state->consumer_key == 0 && prev_kb_state->consumer_key;
}

