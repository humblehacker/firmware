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
#include "blocked_keys.h"

static struct BlockedKeys
{
  bool    keys[NUM_ROWS*NUM_COLS];
  uint8_t length;
} self;


void
BlockedKeys__init()
{
  self.length = sizeof(self.keys)*sizeof(self.keys[0]);
  memset(&self.keys[0], 0, self.length);
}

void
BlockedKeys__block_key(uint8_t cell)
{
  self.keys[cell] = true;
}

void
BlockedKeys__unblock_key(uint8_t cell)
{
  self.keys[cell] = false;
}

bool
BlockedKeys__is_blocked(uint8_t cell)
{
  return self.keys[cell];
}

