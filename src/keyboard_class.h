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

#ifndef __KEYBOARD_CLASS_H__
#define __KEYBOARD_CLASS_H__

#include <LUFA/Drivers/USB/Class/HID.h>

#include "matrix.h"
#include "binding.h"
#include "bound_key.h"

struct USB_KeyboardReport_Data_t;

void      Keyboard__init(void);
void      Keyboard__reset(void);

void      Keyboard__scan_matrix(void);

bool      Keyboard__is_error(void);
uint8_t   Keyboard__fill_report(USB_KeyboardReport_Data_t *report);
bool      Keyboard__is_processing_macro(void);

bool      Keyboard__momentary_mode_engaged(void);
bool      Keyboard__modifier_keys_engaged(void);
void      Keyboard__check_mode_toggle(void);
void      Keyboard__process_keys(void);
void      Keyboard__toggle_map(KeyMap mode_map);

void      Keyboard__update_bindings(void);
void      Keyboard__init_active_keys(void);
BoundKey* Keyboard__first_active_key(void);
BoundKey* Keyboard__next_active_key(void);

#endif // __KEYBOARD_CLASS_H__
