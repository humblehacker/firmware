<%= include_guard(output.path, :begin) %>


#include "config.h"
#include "matrix.h"
<% if map.type == 'system' %>
#include <avr/pgmspace.h>
<% else %>
#include <avr/eeprom.h>
<% end %>
#include <avr/pgmspace.h>

#define NUM_ROWS 8
#define NUM_COLS 18

extern const uint16_t kbd_map_<%=kbIdentifier%>_mx[] <%=storage%>;"
extern const MatrixMap kbd_map_mx_default PROGMEM;

<%= include_guard(output.path, :end) %>
