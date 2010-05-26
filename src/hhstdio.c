#include "hhstdio.h"

static int hid_putc(char, FILE*);
static FILE buf_stdout = FDEV_SETUP_STREAM(hid_putc, NULL, _FDEV_SETUP_WRITE);

#define OUTPUT_BUFSIZE 128
char output_buffer[OUTPUT_BUFSIZE];
uint8_t output_current_pos;

void
init_stdio(void)
{
  stdout = &buf_stdout;
  output_current_pos = 0;
}

int
hid_putc(char c, FILE *stream)
{
  if (output_current_pos == OUTPUT_BUFSIZE - 1)
    return -1;

  output_buffer[output_current_pos] = c;
  output_buffer[++output_current_pos] = '\0';
  return 0;
}

bool
stdout_is_empty(void)
{
  return output_current_pos == 0;
}

char
popchar(void)
{
  if (output_current_pos == OUTPUT_BUFSIZE || !output_buffer[output_current_pos])
  {
    output_buffer[0] = '\0';
    output_current_pos = 0;
    return '\0';
  }

  char ch = output_buffer[output_current_pos];
  if (ch == '\\')
  {
    switch ((ch = output_buffer[++output_current_pos]))
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
  ++output_current_pos;
  return ch;
}



