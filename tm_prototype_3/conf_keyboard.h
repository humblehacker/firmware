
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
#define A_ROWS ((1<<2)|(1<<3)|(1<<6)|(1<<7))
#define C_ROWS ((1<<2)|(1<<3)|(1<<6)|(1<<7))

// Columns
#define D_COLS ((1<<2)|(1<<3)|(1<<6)|(1<<7))
#define F_COLS ((1<<2)|(1<<3)|(1<<6)|(1<<7))
#define E_COLS ((1<<0)|(1<<2)|(1<<3)|(1<<6)|(1<<7))
#define B_COLS ((1<<0)|(1<<2)|(1<<5)|(1<<6)|(1<<7))

static
inline void activate_row(uint8_t row)
{
  // set row pins as outputs
  // TODO: this probably doesn't need to happen every time.
  DDRA &= ~A_ROWS;
  DDRC &= ~C_ROWS;

  // drive all row pins high
  PORTA |= A_ROWS;
  PORTC |= C_ROWS;

  // drive current row pin low
  switch (row)                           // Mylar
  {                                      // -----
    case 0: PORTA &= ~(1 << PA2); break; //   1
    case 1: PORTA &= ~(1 << PA3); break; //  19
    case 2: PORTA &= ~(1 << PA6); break; //  20
    case 3: PORTA &= ~(1 << PA7); break; //  21
    case 4: PORTC &= ~(1 << PC2); break; //  22
    case 5: PORTC &= ~(1 << PC3); break; //  24
    case 6: PORTC &= ~(1 << PC6); break; //  25
    case 7: PORTC &= ~(1 << PC7); break; //  26
  }
}

static
inline uint32_t compress_cols(void)
{
  uint32_t cols = 0;

  //            PIN              Column      Mylar
  //           ------            -------     -----
  if ((~PIND)&(1<<PD2)) cols &= (1UL<< 0); //  2
  if ((~PIND)&(1<<PD3)) cols &= (1UL<< 1); //  3
  if ((~PIND)&(1<<PD6)) cols &= (1UL<< 2); //  4
  if ((~PIND)&(1<<PD7)) cols &= (1UL<< 3); //  5
  if ((~PINF)&(1<<PF2)) cols &= (1UL<< 4); //  6
  if ((~PINF)&(1<<PF3)) cols &= (1UL<< 5); //  7
  if ((~PINF)&(1<<PF6)) cols &= (1UL<< 6); //  8
  if ((~PINF)&(1<<PF7)) cols &= (1UL<< 7); //  9
  if ((~PINE)&(1<<PE0)) cols &= (1UL<< 8); // 10
  if ((~PINE)&(1<<PE2)) cols &= (1UL<< 9); // 11
  if ((~PINE)&(1<<PE3)) cols &= (1UL<<10); // 12
  if ((~PINE)&(1<<PE6)) cols &= (1UL<<11); // 13
  if ((~PINE)&(1<<PE7)) cols &= (1UL<<12); // 14
  if ((~PINB)&(1<<PB0)) cols &= (1UL<<13); // 15
  if ((~PINB)&(1<<PB2)) cols &= (1UL<<14); // 16
  if ((~PINB)&(1<<PB5)) cols &= (1UL<<15); // 17
  if ((~PINB)&(1<<PB6)) cols &= (1UL<<16); // 18
  if ((~PINB)&(1<<PB7)) cols &= (1UL<<17); // 23
  return cols;
}

static
inline void init_cols(void)
{
  /* Columns are inputs */
  DDRD &= ~D_COLS;
  DDRF &= ~F_COLS;
  DDRE &= ~E_COLS;
  DDRB &= ~B_COLS;

  /* Enable pull-up resistors on inputs */
  PORTD  |= D_COLS;
  PORTF  |= F_COLS;
  PORTE  |= E_COLS;
  PORTB  |= B_COLS;
}

// Keyboard LEDs are F0-F1, F4-F5
// TODO: KB_LEDS is currently defined only so it doesn't clash
//       with the ports/pins used for the matrix.  When the
//       LEDs are actually wired, these need to change accordingly.
#define KB_LED_DDR  DDRF
#define KB_LED_PORT PORTF
#define KB_LEDS ((1<<PF0)|(1<<PF1)|(1<<PF4)|(1<<PF5))
#define LED_NUM    (1<<0)
#define LED_CAPS   (1<<1)
#define LED_SCRL   (1<<2)
#define LED_DV     (1<<3)

#endif

#endif // __TYPEMATRIX_PROTOTYPE_2_H__
