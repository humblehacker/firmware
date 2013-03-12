/*
                   The HumbleHacker Keyboard Project
                 Copyright © 2008-2010, David Whetstone
               david DOT whetstone AT humblehacker DOT com
                 Copyright © 2013 Michael Stapelberg
                     michael AT stapelberg DOT de

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

#include "debounce_keys.h"

static struct DebounceKeys
{
  enum { BUFSIZE = NUM_ROWS * NUM_COLS };
  uint8_t count[BUFSIZE];
} self;

void
DebounceKeys__init()
{
  memset(&self.count[0], 0, BUFSIZE);
}

bool
DebounceKeys__pressed(Cell cell)
{
  const int idx = cell_to_index(cell);

  if (self.count[idx] >= 8)
    return true;

  self.count[idx]++;
  return false;
}

void
DebounceKeys__reset(Cell cell)
{
  self.count[cell_to_index(cell)] = 0;
}
