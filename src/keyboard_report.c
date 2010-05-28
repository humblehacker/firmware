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

#include "keyboard_report.h"

void
KeyboardReport__init(KeyboardReport *this)
{
  memset(this, 0, sizeof(KeyboardReport));
}

void
KeyboardReport__copy(KeyboardReport *this, USB_KeyboardReport_Data_t *dst)
{
  memcpy(dst, &this->report, sizeof(*dst));
}


void
KeyboardReport__add_key(KeyboardReport *this, Usage usage)
{
  this->report.KeyCode[this->num_keys] = USAGE_ID(usage);
  ++this->num_keys;
}

void
KeyboardReport__set_modifiers(KeyboardReport *this, uint8_t mod_mask)
{
  this->report.Modifier |= mod_mask;
}

void
KeyboardReport__reset_modifiers(KeyboardReport *this, uint8_t mod_mask)
{
  this->report.Modifier &= ~mod_mask;
}

uint8_t
KeyboardReport__get_modifiers(KeyboardReport *this)
{
  return this->report.Modifier;
}



