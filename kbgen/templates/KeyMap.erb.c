#include "<%= output.path.sub(/\.c$/, '.h')%>"
#include "hid_usages.h"

const Mapping **kbd_map_<%=kbIdentifier%>_mx[] PROGMEM =
{
<% lastcol = flippedMatrix.count
   flippedMatrix.each_index do |iCol| %>

  /* col: <%=iCol%> */<%
     row = flippedMatrix[iCol]
     row.each_index do |iRow|
       location = row[iRow] %>
  /* row:<%=iRow%> loc = <%=location%> */ <%
       key = keymap.keys[location]
       if key == nil %>NULL<%
       elsif key.mappings.empty? == nil %>EMPTY<%
       else%>&<%=kbIdentifier%>_<%=key.location%>[0]<%
       end %><%= "," if iCol < lastcol %><%
     end
   end
%>
};

<% if $keyboard.defaultMap == keymap.id %>
const KeyMap kbd_map_mx_default PROGMEM = &kbd_map_<%=kbIdentifier%>_mx[0];
<% end %>
