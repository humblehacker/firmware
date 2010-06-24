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
<%
rows = cols = 0
  matrix = $keyboard.matrix
  if !matrix.empty? && !matrix[0].empty?
    rows = matrix.size
    cols = matrix[0].size
  end
%>
#define NUM_ROWS <%=rows%>
#define NUM_COLS <%=cols%>

typedef uint8_t Cell;

<% if $keyboard.block_ghost_keys %>
#define BLOCK_GHOST_KEYS
<% end %>

static inline
void
activate_row(uint8_t row)
{
  // set all row pins as inputs
<% $keyboard.rports.each_with_index do |port,i| %>
  DDR<%= port[1,1] %> &= ~(1 << <%= port %>);
<% end %>

  // set current row pin as output
  switch (row)
  {
<% $keyboard.rports.each_with_index do |port,i| %>
    case <%= i %>: DDR<%= $keyboard.rports[i][1,1] %> |= (1 << <%= port %>); break;
<% end %>
  }

  // drive all row pins high
<% $keyboard.rports.each_with_index do |port,i| %>
  PORT<%= port[1,1] %> |= (1 << <%= port %>);
<% end %>

  // drive current row pin low
  switch (row)
  {
<% $keyboard.rports.each_with_index do |port,i| %>
    case <%= i %>: PORT<%= port[1,1] %> &= ~(1 << <%= port %>); break;
<% end %>
  }
}

static inline
uint32_t
read_row_data(void)
{
  uint32_t cols = 0;

<% $keyboard.cports.each_with_index do |port, i| %>
  if ((~PIN<%= port[1,1] %>)&(1<<<%= port %>)) cols |= (1UL<< <%= i %>);
<% end %>

  return cols;
}


static inline
void
init_cols(void)
{
  /* Columns are inputs */
<% $keyboard.cports.each do |port| %>
  DDR<%= port[1,1] %> &= ~(1 << <%= port %>);
<% end %>

  /* Enable pull-up resistors on inputs */
<% $keyboard.cports.each do |port| %>
  PORT<%= port[1,1] %> |= (1 << <%= port %>);
<% end %>
}

#endif /* __MATRIX_H__ */

