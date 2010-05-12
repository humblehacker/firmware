
/*
                    The HumbleHacker Keyboard Project
                    Copyright © 2008, David Whetstone
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

#if defined(TYPEMATRIX_PROTOTYPE_2)

#include "conf_keyboard_defs.h"

#define TARGET_BOARD USBKEY

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
inline uint32_t compress_cols(void)
{
  return ((uint32_t)((~PINE)&E_COLS))          
       | ((uint32_t)((~PINC)&C_COLS)<<( E_BITS        )) 
       | ((uint32_t)((~PIND)&D_COLS)<<( E_BITS+C_BITS ));
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

// Keyboard LEDs are F0-F3
#define KB_LED_DDR  DDRF
#define KB_LED_PORT PORTF
#define KB_LEDS ((1<<0)|(1<<1)|(1<<2)|(1<<3))
#define LED_NUM    (1<<0)
#define LED_CAPS   (1<<1)
#define LED_SCRL   (1<<2)
#define LED_DV     (1<<3)

#endif

#endif // __TYPEMATRIX_PROTOTYPE_2_H__
