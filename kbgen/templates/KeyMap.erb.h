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

<%= include_guard(output.path, :begin) %>


#include "matrix.h"
#include "binding.h"
<% if keymap.type == 'system' %>
#include <avr/pgmspace.h>
<% else %>
#include <avr/eeprom.h>
<% end %>
#include <avr/pgmspace.h>

extern const KeyBindingArray kbd_map_<%=kbIdentifier%>_mx[] <%=storage%>;
extern const KeyMap kbd_map_mx_default PROGMEM;

<%= include_guard(output.path, :end) %>
