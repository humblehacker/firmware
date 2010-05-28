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

#include "registers.h"
#include "matrix_discovery_defs.h"

#define vPINA  0x00
#define vDDRA  0x01
#define vPORTA 0x02

#define vPINB  0x03
#define vDDRB  0x04
#define vPORTB 0x05

#define vPINC  0x06
#define vDDRC  0x07
#define vPORTC 0x08

#define vPIND  0x09
#define vDDRD  0x0A
#define vPORTD 0x0B

#define vPINE  0x0C
#define vDDRE  0x0D
#define vPORTE 0x0E

#define vPINF  0x0F
#define vDDRF  0x10
#define vPORTF 0x11

Registers registers[] =
{
#ifdef USE_PINA0
  { vDDRA, vPORTA, vPINA, (1<<0), "A0" },
#endif
#ifdef USE_PINA1
  { vDDRA, vPORTA, vPINA, (1<<1), "A1" },
#endif
#ifdef USE_PINA2
  { vDDRA, vPORTA, vPINA, (1<<2), "A2" },
#endif
#ifdef USE_PINA3
  { vDDRA, vPORTA, vPINA, (1<<3), "A3" },
#endif
#ifdef USE_PINA4
  { vDDRA, vPORTA, vPINA, (1<<4), "A4" },
#endif
#ifdef USE_PINA5
  { vDDRA, vPORTA, vPINA, (1<<5), "A5" },
#endif
#ifdef USE_PINA6
  { vDDRA, vPORTA, vPINA, (1<<6), "A6" },
#endif
#ifdef USE_PINA7
  { vDDRA, vPORTA, vPINA, (1<<7), "A7" },
#endif

#ifdef USE_PINB0
  { vDDRB, vPORTB, vPINB, (1<<0), "B0" },
#endif
#ifdef USE_PINB1
  { vDDRB, vPORTB, vPINB, (1<<1), "B1" },
#endif
#ifdef USE_PINB2
  { vDDRB, vPORTB, vPINB, (1<<2), "B2" },
#endif
#ifdef USE_PINB3
  { vDDRB, vPORTB, vPINB, (1<<3), "B3" },
#endif
#ifdef USE_PINB4
  { vDDRB, vPORTB, vPINB, (1<<4), "B4" },
#endif
#ifdef USE_PINB5
  { vDDRB, vPORTB, vPINB, (1<<5), "B5" },
#endif
#ifdef USE_PINB6
  { vDDRB, vPORTB, vPINB, (1<<6), "B6" },
#endif
#ifdef USE_PINB7
  { vDDRB, vPORTB, vPINB, (1<<7), "B7" },
#endif

#ifdef USE_PINC0
  { vDDRC, vPORTC, vPINC, (1<<0), "C0" },
#endif
#ifdef USE_PINC1
  { vDDRC, vPORTC, vPINC, (1<<1), "C1" },
#endif
#ifdef USE_PINC2
  { vDDRC, vPORTC, vPINC, (1<<2), "C2" },
#endif
#ifdef USE_PINC3
  { vDDRC, vPORTC, vPINC, (1<<3), "C3" },
#endif
#ifdef USE_PINC4
  { vDDRC, vPORTC, vPINC, (1<<4), "C4" },
#endif
#ifdef USE_PINC5
  { vDDRC, vPORTC, vPINC, (1<<5), "C5" },
#endif
#ifdef USE_PINC6
  { vDDRC, vPORTC, vPINC, (1<<6), "C6" },
#endif
#ifdef USE_PINC7
  { vDDRC, vPORTC, vPINC, (1<<7), "C7" },
#endif

#ifdef USE_PIND0
  { vDDRD, vPORTD, vPIND, (1<<0), "D0" },
#endif
#ifdef USE_PIND1
  { vDDRD, vPORTD, vPIND, (1<<1), "D1" },
#endif
#ifdef USE_PIND2
  { vDDRD, vPORTD, vPIND, (1<<2), "D2" },
#endif
#ifdef USE_PIND3
  { vDDRD, vPORTD, vPIND, (1<<3), "D3" },
#endif
#ifdef USE_PIND4
  { vDDRD, vPORTD, vPIND, (1<<4), "D4" },
#endif
#ifdef USE_PIND5
  { vDDRD, vPORTD, vPIND, (1<<5), "D5" },
#endif
#ifdef USE_PIND6
  { vDDRD, vPORTD, vPIND, (1<<6), "D6" },
#endif
#ifdef USE_PIND7
  { vDDRD, vPORTD, vPIND, (1<<7), "D7" },
#endif

#ifdef USE_PINE0
  { vDDRE, vPORTE, vPINE, (1<<0), "E0" },
#endif
#ifdef USE_PINE1
  { vDDRE, vPORTE, vPINE, (1<<1), "E1" },
#endif
#ifdef USE_PINE2
  { vDDRE, vPORTE, vPINE, (1<<2), "E2" },
#endif
#ifdef USE_PINE3
  { vDDRE, vPORTE, vPINE, (1<<3), "E3" },
#endif
#ifdef USE_PINE4
  { vDDRE, vPORTE, vPINE, (1<<4), "E4" },
#endif
#ifdef USE_PINE5
  { vDDRE, vPORTE, vPINE, (1<<5), "E5" },
#endif
#ifdef USE_PINE6
  { vDDRE, vPORTE, vPINE, (1<<6), "E6" },
#endif
#ifdef USE_PINE7
  { vDDRE, vPORTE, vPINE, (1<<7), "E7" },
#endif

#ifdef USE_PINF0
  { vDDRF, vPORTF, vPINF, (1<<0), "F0" },
#endif
#ifdef USE_PINF1
  { vDDRF, vPORTF, vPINF, (1<<1), "F1" },
#endif
#ifdef USE_PINF2
  { vDDRF, vPORTF, vPINF, (1<<2), "F2" },
#endif
#ifdef USE_PINF3
  { vDDRF, vPORTF, vPINF, (1<<3), "F3" },
#endif
#ifdef USE_PINF4
  { vDDRF, vPORTF, vPINF, (1<<4), "F4" },
#endif
#ifdef USE_PINF5
  { vDDRF, vPORTF, vPINF, (1<<5), "F5" },
#endif
#ifdef USE_PINF6
  { vDDRF, vPORTF, vPINF, (1<<6), "F6" },
#endif
#ifdef USE_PINF7
  { vDDRF, vPORTF, vPINF, (1<<7), "F7" },
#endif
};

uint8_t registers_length = sizeof(registers)/sizeof(Registers);

