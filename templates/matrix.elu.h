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

#ifndef __MATRIX_H__
#define __MATRIX_H__

#include <stdint.h>
#include <avr/io.h>

#define COL(byte) (((uint8_t)byte)>>3)
#define ROW(byte) (((uint8_t)byte)&~(0xff<<3))
#define MATRIX_CELL(row,col) ((((uint8_t)(col))<<3)|((uint8_t)(row)))

#define NUM_ROWS <%= kb.matrix.row_count %>
#define NUM_COLS <%= kb.matrix.col_count %>

typedef uint8_t Cell;

<% if kb.block_ghost_keys then %>
#define BLOCK_GHOST_KEYS
<% end %>

static inline
void
activate_row(uint8_t row)
{
  // set all row pins as inputs
<% for i,pin in ipairs(kb.row_pins) do %>
  DDR<%= string.sub(pin,2,2) %> &= ~(1 << <%= pin %>);
<% end %>

  // set current row pin as output
  switch (row)
  {
<% for i,pin in ipairs(kb.row_pins) do %>
    case <%= i-1 %>: DDR<%= string.sub(pin,2,2) %> |= (1 << <%= pin %>); break;
<% end %>
  }

  // drive all row pins high
<% for i,pin in ipairs(kb.row_pins) do %>
  PORT<%= string.sub(pin,2,2) %> |= (1 << <%= pin %>);
<% end %>

  // drive current row pin low
  switch (row)
  {
<% for i,pin in ipairs(kb.row_pins) do %>
    case <%= i-1 %>: PORT<%= string.sub(pin,2,2) %> &= ~(1 << <%= pin %>); break;
<% end %>
  }
}

static inline
uint32_t
read_row_data(void)
{
  uint32_t cols = 0;

<% for i,pin in ipairs(kb.col_pins) do %>
  if ((~PIN<%= string.sub(pin,2,2) %>)&(1<<<%= pin %>)) cols |= (1UL<< <%= i-1 %>);
<% end %>

  return cols;
}


static inline
void
init_cols(void)
{
  /* Columns are inputs */
<% for i,pin in ipairs(kb.col_pins) do %>
  DDR<%= string.sub(pin,2,2) %> &= ~(1 << <%= pin %>);
<% end %>

  /* Enable pull-up resistors on inputs */
<% for i,pin in ipairs(kb.col_pins) do %>
  PORT<%= string.sub(pin,2,2) %> |= (1 << <%= pin %>);
<% end %>
}

#endif /* __MATRIX_H__ */

