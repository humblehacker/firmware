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

#include "binding.h"

/* All Bindings */
<% $keyboard.maps.each_value do |keymap|
     keymap.keys.each do |location, key|
       key.kbindings.each do |premods, kbinding|
         ident = kbinding_identifier(keymap, key.location, premods, kbinding.class)
         if kbinding.instance_of? Map %>
const MapTarget <%=ident%> PROGMEM = { <%=kbinding.modifiers%>, HID_USAGE_<%=normalize_identifier(kbinding.usage.name)%> };<%
         elsif kbinding.instance_of? Macro %>
const MapTarget <%=ident%>Targets[] PROGMEM =
{
<%         kbinding.kbindings.each do |macro_kbinding| %>
  { <%=macro_kbinding.modifiers%>, HID_USAGE_<%=normalize_identifier(macro_kbinding.usage.name)%> },
<%         end %>
};

const MacroTarget <%= ident %> PROGMEM = { <%=kbinding.kbindings.length%>, &<%=ident%>Targets[0] }; <%
         elsif kbinding.instance_of? Mode %>
const ModeTarget <%= ident %> PROGMEM = { <%=kbinding.type.upcase%>, kbd_map_<%=kbinding.mode%>_mx }; <%
         else
           %><%="/* What? */"%><%
         end
       end
     end
   end
%>

/* Aggregated bindings per key */
<% $keyboard.maps.each_value do |keymap|
     keymap.keys.each do |location, key| %>
const KeyBinding <%= "#{keymap.ids.last}_#{key.location}" %>[] PROGMEM =
{<%    key.kbindings.each do |premods, kbinding| %>
  { <%   if kbinding.instance_of? Map
    %>MAP, <%
         elsif kbinding.instance_of? Macro
    %>MACRO, <%
         elsif kbinding.instance_of? Mode
    %>MODE, <%
         else
    %><%="/* What? */"%>, <%
         end
%><%= premods %>, (void*)&<%= kbinding_identifier(keymap, key.location, premods, kbinding.class) %> }, <%
       end %>
};
<%
     end
   end
%>


