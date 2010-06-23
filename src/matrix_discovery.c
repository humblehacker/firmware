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

extern const USB_KeyboardReport_Data_t ascii_table[] PROGMEM;

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
    memcpy_P(report, &ascii_table[0], sizeof(USB_KeyboardReport_Data_t));
    self.send_empty_report = false;
    return;
  }

  char ch = stdout_popchar();
  if (!ch)
    return;

  memcpy_P(report, &ascii_table[(uint8_t)ch], sizeof(USB_KeyboardReport_Data_t));
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
    return;
  }

  if (!stdout_is_empty())
    write_output_char(report);

  return sizeof(USB_KeyboardReport_Data_t);
}

const USB_KeyboardReport_Data_t ascii_table[] =
{
  /* 0   .  */ { NONE, 0, { USAGE_ID(HID_USAGE_NONE) }},
  /* 1   .  */ { NONE, 0, { USAGE_ID(HID_USAGE_NONE) }},
  /* 2   .  */ { NONE, 0, { USAGE_ID(HID_USAGE_NONE) }},
  /* 3   .  */ { NONE, 0, { USAGE_ID(HID_USAGE_NONE) }},
  /* 4   .  */ { NONE, 0, { USAGE_ID(HID_USAGE_NONE) }},
  /* 5   .  */ { NONE, 0, { USAGE_ID(HID_USAGE_NONE) }},
  /* 6   .  */ { NONE, 0, { USAGE_ID(HID_USAGE_NONE) }},
  /* 7   .  */ { NONE, 0, { USAGE_ID(HID_USAGE_NONE) }},
  /* 8   \b */ { NONE, 0, { USAGE_ID(HID_USAGE_BACKSPACE) }},
  /* 9   \t */ { NONE, 0, { USAGE_ID(HID_USAGE_TAB) }},
  /* 10  \n */ { NONE, 0, { USAGE_ID(HID_USAGE_RETURN_OR_ENTER) }},
  /* 11  .  */ { NONE, 0, { USAGE_ID(HID_USAGE_NONE) }},
  /* 12  .  */ { NONE, 0, { USAGE_ID(HID_USAGE_NONE) }},
  /* 13  .  */ { NONE, 0, { USAGE_ID(HID_USAGE_NONE) }},
  /* 14  .  */ { NONE, 0, { USAGE_ID(HID_USAGE_NONE) }},
  /* 15  .  */ { NONE, 0, { USAGE_ID(HID_USAGE_NONE) }},

  /* 16  . */ { NONE, 0, { USAGE_ID(HID_USAGE_NONE) }},
  /* 17  . */ { NONE, 0, { USAGE_ID(HID_USAGE_NONE) }},
  /* 18  . */ { NONE, 0, { USAGE_ID(HID_USAGE_NONE) }},
  /* 19  . */ { NONE, 0, { USAGE_ID(HID_USAGE_NONE) }},
  /* 20  . */ { NONE, 0, { USAGE_ID(HID_USAGE_NONE) }},
  /* 21  . */ { NONE, 0, { USAGE_ID(HID_USAGE_NONE) }},
  /* 22  . */ { NONE, 0, { USAGE_ID(HID_USAGE_NONE) }},
  /* 23  . */ { NONE, 0, { USAGE_ID(HID_USAGE_NONE) }},
  /* 24  . */ { NONE, 0, { USAGE_ID(HID_USAGE_NONE) }},
  /* 25  . */ { NONE, 0, { USAGE_ID(HID_USAGE_NONE) }},
  /* 26  . */ { NONE, 0, { USAGE_ID(HID_USAGE_NONE) }},
  /* 27  . */ { NONE, 0, { USAGE_ID(HID_USAGE_NONE) }},
  /* 28  . */ { NONE, 0, { USAGE_ID(HID_USAGE_NONE) }},
  /* 29  . */ { NONE, 0, { USAGE_ID(HID_USAGE_NONE) }},
  /* 30  . */ { NONE, 0, { USAGE_ID(HID_USAGE_NONE) }},
  /* 31  . */ { NONE, 0, { USAGE_ID(HID_USAGE_NONE) }},

  /* 32    */ { NONE, 0, { USAGE_ID(HID_USAGE_SPACEBAR) }},
  /* 33  ! */ { L_SHF, 0, { USAGE_ID(HID_USAGE_1_AND__EXCLAMATION) }},
  /* 34  " */ { L_SHF, 0, { USAGE_ID(HID_USAGE__APOSTROPHE_AND__QUOTE) }},
  /* 35  # */ { L_SHF, 0, { USAGE_ID(HID_USAGE_3_AND__POUND)  }},
  /* 36  $ */ { L_SHF, 0, { USAGE_ID(HID_USAGE_4_AND__DOLLAR) }},
  /* 37  % */ { L_SHF, 0, { USAGE_ID(HID_USAGE_5_AND__PERCENT) }},
  /* 38  & */ { L_SHF, 0, { USAGE_ID(HID_USAGE_7_AND__AMPERSAND) }},
  /* 39  ' */ { NONE, 0, { USAGE_ID(HID_USAGE__APOSTROPHE_AND__QUOTE) }},
  /* 40  ( */ { L_SHF, 0, { USAGE_ID(HID_USAGE_9_AND__LPAREN) }},
  /* 41  ) */ { L_SHF, 0, { USAGE_ID(HID_USAGE_0_AND__RPAREN) }},
  /* 42  * */ { L_SHF, 0, { USAGE_ID(HID_USAGE_8_AND__ASTERISK) }},
  /* 43  + */ { L_SHF, 0, { USAGE_ID(HID_USAGE__EQUALS_AND__PLUS) }},
  /* 44  , */ { NONE, 0, { USAGE_ID(HID_USAGE__COMMA_AND__LESSTHAN) }},
  /* 45  - */ { NONE, 0, { USAGE_ID(HID_USAGE__MINUS_AND_UNDERSCORE) }},
  /* 46  . */ { NONE, 0, { USAGE_ID(HID_USAGE__PERIOD_AND__GREATERTHAN) }},
  /* 47  / */ { NONE, 0, { USAGE_ID(HID_USAGE__SLASH_AND__QUESTION) }},

  /* 48  0 */ { NONE, 0, { USAGE_ID(HID_USAGE_0_AND__RPAREN) }},
  /* 49  1 */ { NONE, 0, { USAGE_ID(HID_USAGE_1_AND__EXCLAMATION) }},
  /* 50  2 */ { NONE, 0, { USAGE_ID(HID_USAGE_2_AND__ATSIGN)      }},
  /* 51  3 */ { NONE, 0, { USAGE_ID(HID_USAGE_3_AND__POUND)       }},
  /* 52  4 */ { NONE, 0, { USAGE_ID(HID_USAGE_4_AND__DOLLAR)      }},
  /* 53  5 */ { NONE, 0, { USAGE_ID(HID_USAGE_5_AND__PERCENT)     }},
  /* 54  6 */ { NONE, 0, { USAGE_ID(HID_USAGE_6_AND__CARET)       }},
  /* 55  7 */ { NONE, 0, { USAGE_ID(HID_USAGE_7_AND__AMPERSAND)   }},
  /* 56  8 */ { NONE, 0, { USAGE_ID(HID_USAGE_8_AND__ASTERISK)    }},
  /* 57  9 */ { NONE, 0, { USAGE_ID(HID_USAGE_9_AND__LPAREN)      }},
  /* 58  : */ { L_SHF, 0, { USAGE_ID(HID_USAGE__SEMICOLON_AND__COLON) }},
  /* 59  ; */ { NONE, 0, { USAGE_ID(HID_USAGE__SEMICOLON_AND__COLON) }},
  /* 60  < */ { L_SHF, 0, { USAGE_ID(HID_USAGE__COMMA_AND__LESSTHAN) }},
  /* 61  = */ { NONE, 0, { USAGE_ID(HID_USAGE__EQUALS_AND__PLUS) }},
  /* 62  > */ { L_SHF, 0, { USAGE_ID(HID_USAGE__PERIOD_AND__GREATERTHAN) }},
  /* 63  ? */ { L_SHF, 0, { USAGE_ID(HID_USAGE__SLASH_AND__QUESTION) }},

  /* 64  @ */ { L_SHF, 0, { USAGE_ID(HID_USAGE_2_AND__ATSIGN) }},
  /* 65  A */ { L_SHF, 0, { USAGE_ID(HID_USAGE_A_AND_A) }},
  /* 66  B */ { L_SHF, 0, { USAGE_ID(HID_USAGE_B_AND_B) }},
  /* 67  C */ { L_SHF, 0, { USAGE_ID(HID_USAGE_C_AND_C) }},
  /* 68  D */ { L_SHF, 0, { USAGE_ID(HID_USAGE_D_AND_D) }},
  /* 69  E */ { L_SHF, 0, { USAGE_ID(HID_USAGE_E_AND_E) }},
  /* 70  F */ { L_SHF, 0, { USAGE_ID(HID_USAGE_F_AND_F) }},
  /* 71  G */ { L_SHF, 0, { USAGE_ID(HID_USAGE_G_AND_G) }},
  /* 72  H */ { L_SHF, 0, { USAGE_ID(HID_USAGE_H_AND_H) }},
  /* 73  I */ { L_SHF, 0, { USAGE_ID(HID_USAGE_I_AND_I) }},
  /* 74  J */ { L_SHF, 0, { USAGE_ID(HID_USAGE_J_AND_J) }},
  /* 75  K */ { L_SHF, 0, { USAGE_ID(HID_USAGE_K_AND_K) }},
  /* 76  L */ { L_SHF, 0, { USAGE_ID(HID_USAGE_L_AND_L) }},
  /* 77  M */ { L_SHF, 0, { USAGE_ID(HID_USAGE_M_AND_M) }},
  /* 78  N */ { L_SHF, 0, { USAGE_ID(HID_USAGE_N_AND_N) }},
  /* 79  O */ { L_SHF, 0, { USAGE_ID(HID_USAGE_O_AND_O) }},

  /* 80  P */ { L_SHF, 0, { USAGE_ID(HID_USAGE_P_AND_P) }},
  /* 81  Q */ { L_SHF, 0, { USAGE_ID(HID_USAGE_Q_AND_Q) }},
  /* 82  R */ { L_SHF, 0, { USAGE_ID(HID_USAGE_R_AND_R) }},
  /* 83  S */ { L_SHF, 0, { USAGE_ID(HID_USAGE_S_AND_S) }},
  /* 84  T */ { L_SHF, 0, { USAGE_ID(HID_USAGE_T_AND_T) }},
  /* 85  U */ { L_SHF, 0, { USAGE_ID(HID_USAGE_U_AND_U) }},
  /* 86  V */ { L_SHF, 0, { USAGE_ID(HID_USAGE_V_AND_V) }},
  /* 87  W */ { L_SHF, 0, { USAGE_ID(HID_USAGE_W_AND_W) }},
  /* 88  X */ { L_SHF, 0, { USAGE_ID(HID_USAGE_X_AND_X) }},
  /* 89  Y */ { L_SHF, 0, { USAGE_ID(HID_USAGE_Y_AND_Y) }},
  /* 90  Z */ { L_SHF, 0, { USAGE_ID(HID_USAGE_Z_AND_Z) }},
  /* 91  [ */ { NONE, 0, { USAGE_ID(HID_USAGE__LSQUAREBRACKET_AND__LCURLYBRACE) }},
  /* 92  \ */ { NONE, 0, { USAGE_ID(HID_USAGE__BACKSLASH_AND__PIPE) }},
  /* 93  ] */ { NONE, 0, { USAGE_ID(HID_USAGE__RSQUAREBRACKET_AND__RCURLYBRACE) }},
  /* 94  ^ */ { L_SHF, 0, { USAGE_ID(HID_USAGE_6_AND__CARET) }},
  /* 95  _ */ { L_SHF, 0, { USAGE_ID(HID_USAGE__MINUS_AND_UNDERSCORE) }},

  /* 96  ` */ { NONE, 0, { USAGE_ID(HID_USAGE_NONE) }},
  /* 97  a */ { NONE, 0, { USAGE_ID(HID_USAGE_A_AND_A) }},
  /* 98  b */ { NONE, 0, { USAGE_ID(HID_USAGE_B_AND_B) }},
  /* 99  c */ { NONE, 0, { USAGE_ID(HID_USAGE_C_AND_C) }},
  /* 100 d */ { NONE, 0, { USAGE_ID(HID_USAGE_D_AND_D) }},
  /* 101 e */ { NONE, 0, { USAGE_ID(HID_USAGE_E_AND_E) }},
  /* 102 f */ { NONE, 0, { USAGE_ID(HID_USAGE_F_AND_F) }},
  /* 103 g */ { NONE, 0, { USAGE_ID(HID_USAGE_G_AND_G) }},
  /* 104 h */ { NONE, 0, { USAGE_ID(HID_USAGE_H_AND_H) }},
  /* 105 i */ { NONE, 0, { USAGE_ID(HID_USAGE_I_AND_I) }},
  /* 106 j */ { NONE, 0, { USAGE_ID(HID_USAGE_J_AND_J) }},
  /* 107 k */ { NONE, 0, { USAGE_ID(HID_USAGE_K_AND_K) }},
  /* 108 l */ { NONE, 0, { USAGE_ID(HID_USAGE_L_AND_L) }},
  /* 109 m */ { NONE, 0, { USAGE_ID(HID_USAGE_M_AND_M) }},
  /* 110 n */ { NONE, 0, { USAGE_ID(HID_USAGE_N_AND_N) }},
  /* 111 o */ { NONE, 0, { USAGE_ID(HID_USAGE_O_AND_O) }},

  /* 112 p */ { NONE, 0, { USAGE_ID(HID_USAGE_P_AND_P) }},
  /* 113 q */ { NONE, 0, { USAGE_ID(HID_USAGE_Q_AND_Q) }},
  /* 114 r */ { NONE, 0, { USAGE_ID(HID_USAGE_R_AND_R) }},
  /* 115 s */ { NONE, 0, { USAGE_ID(HID_USAGE_S_AND_S) }},
  /* 116 t */ { NONE, 0, { USAGE_ID(HID_USAGE_T_AND_T) }},
  /* 117 u */ { NONE, 0, { USAGE_ID(HID_USAGE_U_AND_U) }},
  /* 118 v */ { NONE, 0, { USAGE_ID(HID_USAGE_V_AND_V) }},
  /* 119 w */ { NONE, 0, { USAGE_ID(HID_USAGE_W_AND_W) }},
  /* 120 x */ { NONE, 0, { USAGE_ID(HID_USAGE_X_AND_X) }},
  /* 121 y */ { NONE, 0, { USAGE_ID(HID_USAGE_Y_AND_Y) }},
  /* 122 z */ { NONE, 0, { USAGE_ID(HID_USAGE_Z_AND_Z) }},
  /* 123 { */ { L_SHF, 0, { USAGE_ID(HID_USAGE__LSQUAREBRACKET_AND__LCURLYBRACE) }},
  /* 124 | */ { L_SHF, 0, { USAGE_ID(HID_USAGE__BACKSLASH_AND__PIPE) }},
  /* 125 } */ { L_SHF, 0, { USAGE_ID(HID_USAGE__RSQUAREBRACKET_AND__RCURLYBRACE) }},
  /* 126 ~ */ { L_SHF, 0, { USAGE_ID(HID_USAGE__BACKTICK_AND__TILDE) }},
};


