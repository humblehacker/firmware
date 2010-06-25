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

#include <util/delay.h>
#include <stdio.h>

#include "matrix_discovery.h"
#include "ioports.h"
#include "hhstdio.h"

struct MatrixDiscovery
{
  enum State { LEARN, DISPLAY, AWAITING_INPUT, IDLE } state;
  bool send_empty_report;
  bool welcome_sent;
} self;

static void write_output_char(USB_KeyboardReport_Data_t *report);
static void activate_row(int row);
static bool check_column(int col);
static void reset_pins(void);

void
MatrixDiscovery__init()
{
  stdio_init();
  self.send_empty_report = false;
  self.welcome_sent = false;
}

void
write_output_char(USB_KeyboardReport_Data_t *report)
{
  if (self.send_empty_report)
  {
    stdio_fill_report('\0', report);
    self.send_empty_report = false;
    return;
  }

  char ch = stdout_popchar();
  if (!ch)
    return;

  stdio_fill_report(ch, report);
  self.send_empty_report = true;
}

void
reset_pins()
{
  PORTA |= PORTA_MASK;
  PORTB |= PORTB_MASK;
  PORTC |= PORTC_MASK;
  PORTD |= PORTD_MASK;
  PORTE |= PORTE_MASK;
  PORTF |= PORTF_MASK;
}

void
activate_row(int row)
{
  IOPort *port = &ioports[row];

  DDR(port)  |=  port->bitmask;  // 1 = pin as output
  PORT(port) &= ~port->bitmask;  // 0 = drive pin low
  _delay_us(20);
}

bool
check_column(int col)
{
  IOPort *port = &ioports[col];

  DDR(port)  &= ~port->bitmask;  // 0 = pin as input
  PORT(port) |=  port->bitmask;  // 1 = activate pull-up

  return (~PIN(port)) & port->bitmask;
}

void
MatrixDiscovery__scan_matrix()
{
  if (!stdout_is_empty())
    return;

  enum {MAX_VERTICES=20};
  static struct
  {
    int row, col;
  } vertices[MAX_VERTICES];
  int vertex = 0;

  int col_start = 0;
  for (int row = 0; row < ioports_length; ++row)
  {
    for (int col = col_start++; col < ioports_length; ++col)
    {
      reset_pins();
      activate_row(row);
      if (row != col && check_column(col))
      {
        vertices[vertex].row = row;
        vertices[vertex].col = col;
        ++vertex;
        if (vertex > MAX_VERTICES)
          break;
      }
    }
  }
  if (vertex)
  {
    for (int x = 0; x < vertex; ++x)
      printf("(%s, %s), ", ioports[vertices[x].row].name, ioports[vertices[x].col].name);
    printf("\n");
  }
}

uint8_t
MatrixDiscovery__get_report(USB_KeyboardReport_Data_t *report)
{
  if (!self.welcome_sent)
  {
    _delay_ms(10000);
    printf("The HumbleHacker Keyboard firmware\n");
    printf("------ Matrix Discovery Mode -----\n\n");
    printf("Press any key to find out which PORT/PIN it is connected to.\n");
    self.welcome_sent = true;
    return 0;
  }

  if (!stdout_is_empty())
    write_output_char(report);

  return sizeof(USB_KeyboardReport_Data_t);
}



