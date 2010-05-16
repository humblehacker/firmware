#include "<%= output.path.sub(/\.c$/, '.h')%>"
#include "HIDUsageTables.h"
#include "ModeKeys.h"

const uint16_t kbd_map_Common_mx[] PROGMEM =
{
<% lastcol = flippedMatrix.count
   flippedMatrix.each_index do |iCol| %>

  /* col: <%=iCol%> */
<%   row = flippedMatrix[iCol]
     row.each_index do |iRow|
       location = row[iRow] %>
  /* row:<%=iRow%> loc = <%=location%> */ <%
       key = map.keys[location]
       if key == nil %>NIL<%
       elsif key.mappings.empty? == nil %>EMPTY<%
       else %>MAPPING<%
         key.mappings.each do |premods, mapping|
           %>::<%=premods%>:<%
           if mapping.instance_of? Map
             if mapping.usage != nil
               %><%=mapping.usage.page.name%>:<%=mapping.usage.name%>, <%
             else
               %><%="what happened?"%><%
             end
           else
             %><%=mapping.class%>,<%
           end
         end
       end %><%= "," if iCol < lastcol %><%
     end
   end
%>

<% if $keyboard.defaultMap == map.id %>
const MatrixMap kbd_map_mx_default PROGMEM = kbd_map_<%=kbIdentifier%>_mx;
<% end %>

<%#

< %     elsif !key.macro.empty? % >
          HID_USAGE_MACRO(<%=macroIndex% >)
< %        macroIndex += 1
        elsif key.mode != '' % >
          MAKE_USAGE(HID_USAGE_PAGE_CUSTOM, MODE_KEY_<%=normalize_identifier(key.mode)% >)
< %      else % >
          HID_USAGE_<%=normalize_identifier(key.usage.name)% >

%>
