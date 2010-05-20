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

#ifndef __TYPEMATRIX_PROTOTYPE_2_H__
#define __TYPEMATRIX_PROTOTYPE_2_H__

// Rows are A0-A7
#define ROW_DDR  DDRA
#define ROW_PORT PORTA
#define A_ROWS 0xff

// Columns are E0-E1, C0-C7, D0-D7
#define E_COLS ((1<<0)|(1<<1))
#define E_BITS 2
#define C_COLS 0xff
#define C_BITS 8
#define D_COLS 0xff

static
inline void activate_row(uint8_t row)
{
  // Activate the current row
  ROW_DDR  =  (1 << row);  // set row as output
  ROW_PORT = ~(1 << row);  // row pin driven low
}

static
inline
uint32_t
fix(uint32_t val)
{
  // While wiring up the controller board, columns
  // 7 & 9 and 10 & 12 were inadvertently transposed,
  // so we put them back in their correct order here.

  uint32_t bit7  = val & (1<<7);
  uint32_t bit9  = val & (1<<9);
  uint32_t bit10 = val & (1<<10);
  uint32_t bit12 = val & (1<<12);

  val &= ~((1<<12)|(1<<10)|(1<<9)|(1<<7));
  val |= ((bit10<<2)|(bit12>>2)|(bit7<<2)|(bit9>>2));
  return val;
}

static
inline
uint32_t
read_row_data(void)
{
  uint32_t result;
  result = ((uint32_t)((~PINE)&E_COLS))
         | ((uint32_t)((~PINC)&C_COLS)<<( E_BITS        ))
         | ((uint32_t)((~PIND)&D_COLS)<<( E_BITS+C_BITS ));
  return fix(result);
}

static
inline void init_cols(void)
{
  /* Columns are inputs */
  DDRE &= ~E_COLS;
  DDRC  = 0;
  DDRD  = 0;

  /* Enable pull-up resistors on inputs */
  PORTE  |= E_COLS;
  PORTC   = C_COLS;
  PORTD   = D_COLS;
}

#endif // __TYPEMATRIX_PROTOTYPE_2_H__
