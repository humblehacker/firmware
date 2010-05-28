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

#include "hhstdio.h"

static int hid_putc(char, FILE*);
static FILE buf_stdout = FDEV_SETUP_STREAM(hid_putc, NULL, _FDEV_SETUP_WRITE);

#define OUTPUT_BUFSIZE 256
char output_buffer[OUTPUT_BUFSIZE];
uint8_t current_pos;
uint8_t end_pos;

void
stdio_init(void)
{
  stdout      = &buf_stdout;
  current_pos = 0;
  end_pos     = 0;
}

int
hid_putc(char c, FILE *stream)
{
  if (end_pos == OUTPUT_BUFSIZE - 1)
    return -1;

  output_buffer[end_pos] = c;
  output_buffer[++end_pos] = '\0';
  return 0;
}

bool
stdout_is_empty(void)
{
  return end_pos == 0;
}

char
stdout_popchar(void)
{
  if (current_pos == OUTPUT_BUFSIZE || !output_buffer[current_pos])
  {
    output_buffer[0] = '\0';
    current_pos = 0;
    end_pos = 0;
    return '\0';
  }

  char ch = output_buffer[current_pos];
  if (ch == '\\')
  {
    switch ((ch = output_buffer[++current_pos]))
    {
    case 'n':
      ch = (char)10; break;
    case 't':
      ch = (char)9;  break;
    case 'b':
      ch = (char)8;  break;
    case '\\':
      break;
    }
  }
  ++current_pos;
  return ch;
}



