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

#endif // __REGISTERS_H__
