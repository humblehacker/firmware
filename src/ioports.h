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

#ifndef __IOPORTS_H__
#define __IOPORTS_H__

#include <avr/pgmspace.h>

struct IOPort
{
  uint8_t ddr;
  uint8_t port;
  uint8_t pin;
  uint8_t bitmask;
  char name[3];
};

typedef struct IOPort IOPort;

extern uint8_t ioports_length;
extern IOPort ioports[];

#define PIN(p)  _SFR_IO8(p->pin)
#define DDR(p)  _SFR_IO8(p->ddr)
#define PORT(p) _SFR_IO8(p->port)

enum
{
  PORTA_MASK =
#ifdef USE_PINA0
    (1<<0) |
#endif
#ifdef USE_PINA1
    (1<<1) |
#endif
#ifdef USE_PINA2
    (1<<2) |
#endif
#ifdef USE_PINA3
    (1<<3) |
#endif
#ifdef USE_PINA4
    (1<<4) |
#endif
#ifdef USE_PINA5
    (1<<5) |
#endif
#ifdef USE_PINA6
    (1<<6) |
#endif
#ifdef USE_PINA7
    (1<<7) |
#endif
    0,
  PORTB_MASK =
#ifdef USE_PINB0
    (1<<0) |
#endif
#ifdef USE_PINB1
    (1<<1) |
#endif
#ifdef USE_PINB2
    (1<<2) |
#endif
#ifdef USE_PINB3
    (1<<3) |
#endif
#ifdef USE_PINB4
    (1<<4) |
#endif
#ifdef USE_PINB5
    (1<<5) |
#endif
#ifdef USE_PINB6
    (1<<6) |
#endif
#ifdef USE_PINB7
    (1<<7) |
#endif
    0,
  PORTC_MASK =
#ifdef USE_PINC0
    (1<<0) |
#endif
#ifdef USE_PINC1
    (1<<1) |
#endif
#ifdef USE_PINC2
    (1<<2) |
#endif
#ifdef USE_PINC3
    (1<<3) |
#endif
#ifdef USE_PINC4
    (1<<4) |
#endif
#ifdef USE_PINC5
    (1<<5) |
#endif
#ifdef USE_PINC6
    (1<<6) |
#endif
#ifdef USE_PINC7
    (1<<7) |
#endif
    0,
  PORTD_MASK =
#ifdef USE_PIND0
    (1<<0) |
#endif
#ifdef USE_PIND1
    (1<<1) |
#endif
#ifdef USE_PIND2
    (1<<2) |
#endif
#ifdef USE_PIND3
    (1<<3) |
#endif
#ifdef USE_PIND4
    (1<<4) |
#endif
#ifdef USE_PIND5
    (1<<5) |
#endif
#ifdef USE_PIND6
    (1<<6) |
#endif
#ifdef USE_PIND7
    (1<<7) |
#endif
    0,
  PORTE_MASK =
#ifdef USE_PINE0
    (1<<0) |
#endif
#ifdef USE_PINE1
    (1<<1) |
#endif
#ifdef USE_PINE2
    (1<<2) |
#endif
#ifdef USE_PINE3
    (1<<3) |
#endif
#ifdef USE_PINE4
    (1<<4) |
#endif
#ifdef USE_PINE5
    (1<<5) |
#endif
#ifdef USE_PINE6
    (1<<6) |
#endif
#ifdef USE_PINE7
    (1<<7) |
#endif
    0,
  PORTF_MASK =
#ifdef USE_PINF0
    (1<<0) |
#endif
#ifdef USE_PINF1
    (1<<1) |
#endif
#ifdef USE_PINF2
    (1<<2) |
#endif
#ifdef USE_PINF3
    (1<<3) |
#endif
#ifdef USE_PINF4
    (1<<4) |
#endif
#ifdef USE_PINF5
    (1<<5) |
#endif
#ifdef USE_PINF6
    (1<<6) |
#endif
#ifdef USE_PINF7
    (1<<7) |
#endif
    0,
};



#endif // __IOPORTS_H__
