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

#endif /* __MATRIX_H__ */

