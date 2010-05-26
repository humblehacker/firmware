#ifndef __REGISTERS_H__
#define __REGISTERS_H__

#include <avr/pgmspace.h>

typedef struct
{
  uint8_t ddr;
  uint8_t port;
  uint8_t pin;
  uint8_t bitmask;
  char name[3];
} Registers;

extern uint8_t registers_length;
extern Registers registers[];

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


#endif // __REGISTERS_H__
