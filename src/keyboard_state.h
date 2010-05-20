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

#ifndef __KEYBOARD_STATE_H__
#define __KEYBOARD_STATE_H__

#include "matrix.h"
#include "mapping.h"

#define NUM_MODIFIERS 8
#define MAX_KEYS      6
#define MAX_ACTIVE_CELLS (MAX_KEYS + NUM_MODIFIERS)
//#define MAX_ACTIVE_CELLS (MAX_KEYS + NUM_MODE_KEYS + NUM_MODIFIERS)
enum {DEACTIVATED=((uint8_t)-1)};

typedef struct
{
  Modifiers modifiers;
  Usage keys[MAX_KEYS];
  uint8_t num_keys;
  Cell active_cells[MAX_ACTIVE_CELLS];
  uint8_t num_active_cells;
  const MacroTarget *macro;
  bool error_roll_over;
} KeyboardState;

extern KeyboardState g_kb_state;
extern uint8_t g_num_blocked_keys;
extern uint8_t g_blocked_keys[MAX_ACTIVE_CELLS];

void keyboard_state__reset(void);

uint8_t   keyboard_state__is_error(void);
uint8_t   keyboard_state__is_empty(void);
uint8_t   keyboard_state__is_processing_macro(void);

#endif // __KEYBOARD_STATE_H__
