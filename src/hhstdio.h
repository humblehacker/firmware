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

#ifndef __HHSTDIO_H__
#define __HHSTDIO_H__

#include <stdio.h>
#include <stdbool.h>
#include <LUFA/Drivers/USB/Class/HID.h>

#ifdef USE_HHSTDIO

void stdio_init(void);
void stdio_fill_report(char ch, USB_KeyboardReport_Data_t *report);
bool stdout_is_empty(void);
char stdout_popchar(void);
extern const USB_KeyboardReport_Data_t ascii_table[] PROGMEM;

#else

inline void stdio_init(void) {}
inline void stdio_fill_report(char ch, USB_KeyboardReport_Data_t *report) {}
inline bool stdout_is_empty(void) { return true; }
inline char stdout_popchar(void) { return '\0'; }

#endif

#endif // __HHSTDIO_H__
