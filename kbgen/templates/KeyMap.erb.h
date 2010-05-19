<%= include_guard(output.path, :begin) %>


#include "matrix.h"
#include "mapping.h"
<% if keymap.type == 'system' %>
#include <avr/pgmspace.h>
<% else %>
#include <avr/eeprom.h>
<% end %>
#include <avr/pgmspace.h>

#define NUM_ROWS 8
#define NUM_COLS 18

extern const KeyMappingArray kbd_map_<%=kbIdentifier%>_mx[] <%=storage%>;
extern const KeyMap kbd_map_mx_default PROGMEM;

<%= include_guard(output.path, :end) %>
