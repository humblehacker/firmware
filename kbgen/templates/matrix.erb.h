#ifndef __MATRIX_H__
#define __MATRIX_H__

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

typedef const uint16_t * MatrixMap;
typedef uint8_t Cell;

extern const MatrixMap kbd_map_mx_default;

#endif /* __MATRIX_H__ */

