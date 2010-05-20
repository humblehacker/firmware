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

#include "<%= output.path.sub(/\.c$/, '.h')%>"
#include "hid_usages.h"

const KeyMappingArray kbd_map_<%=kbIdentifier%>_mx[] PROGMEM =
{
<% lastcol = flippedMatrix.count
   flippedMatrix.each_index do |iCol| %>

  /* col: <%=iCol%> */<%
     row = flippedMatrix[iCol]
     row.each_index do |iRow|
       location = row[iRow] %>
  /* row:<%=iRow%> loc = <%=location%> */ <%
       key = keymap.keys[location]
       if key == nil %>{0, NULL}<%
       elsif key.mappings.empty? == nil %>{0, NULL} /* EMPTY DEFINITION! */<%
       else%>{<%=key.mappings.length%>, &<%=kbIdentifier%>_<%=key.location%>[0]}<%
       end %><%= "," if iCol < lastcol %><%
     end
   end
%>
};

<% if $keyboard.defaultMap == keymap.id %>
const KeyMap kbd_map_mx_default PROGMEM = &kbd_map_<%=kbIdentifier%>_mx[0];
<% end %>
