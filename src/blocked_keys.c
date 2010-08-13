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
  enum { BUFSIZE = NUM_ROWS * NUM_COLS };
  bool keys[BUFSIZE];
} self;


void
BlockedKeys__init()
{
  memset(&self.keys[0], 0, BUFSIZE);
}

void
BlockedKeys__block_key(Cell cell)
{
  self.keys[cell_to_index(cell)] = true;
}

void
BlockedKeys__unblock_key(Cell cell)
{
  self.keys[cell_to_index(cell)] = false;
}

bool
BlockedKeys__is_blocked(Cell cell)
{
  return self.keys[cell_to_index(cell)];
}

