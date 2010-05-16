#include "<%= output.path.sub(/\.c$/, '.h')%>"
#include "hid_usages.h"
#include "ModeKeys.h"

const Mapping* kbd_map_Common_mx[] PROGMEM =
{
<% lastcol = flippedMatrix.count
   flippedMatrix.each_index do |iCol| %>

  /* col: <%=iCol%> */<%
     row = flippedMatrix[iCol]
     row.each_index do |iRow|
       location = row[iRow] %>
  /* row:<%=iRow%> loc = <%=location%> */ <%
       key = keymap.keys[location]
       if key == nil %>NIL<%
       elsif key.mappings.empty? == nil %>EMPTY<%
       else%><%="#{keymap.ids.last}_#{key.location}"%><%
       end %><%= "," if iCol < lastcol %><%
     end
   end
%>
};

<% if $keyboard.defaultMap == keymap.id %>
const MatrixMap kbd_map_mx_default PROGMEM = kbd_map_<%=kbIdentifier%>_mx;
<% end %>
