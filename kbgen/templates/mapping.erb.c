#include "mapping.h"

/* All Mappings */
<% $keyboard.maps.each_value do |keymap|
     keymap.keys.each do |location, key|
       key.mappings.each do |premods, mapping|
         ident = mapping_identifier(keymap, key.location, premods, mapping.class)
         if mapping.instance_of? Map %>
const MapTarget <%=ident%> = { <%=mapping.modifiers%>, HID_USAGE_<%=normalize_identifier(mapping.usage.name)%> };<%
         elsif mapping.instance_of? Macro %>
const MapTarget <%=ident%>Targets[] =
{
<%         mapping.mappings.each do |macro_mapping| %>
  { <%=macro_mapping.modifiers%>, HID_USAGE_<%=normalize_identifier(macro_mapping.usage.name)%> },
<%         end %>
};

const MacroTarget <%= ident %> = { <%=mapping.mappings.length%>, &<%=ident%>Targets[0] }; <%
         elsif mapping.instance_of? Mode %>
const ModeTarget <%= ident %> = { <%=mapping.type.upcase%>, kbd_map_<%=mapping.mode%>_mx }; <%
         else
           %><%="/* What? */"%><%
         end
       end
     end
   end
%>

/* Aggregated mappings per key */
<% $keyboard.maps.each_value do |keymap|
     keymap.keys.each do |location, key| %>
const KeyMapping <%= "#{keymap.ids.last}_#{key.location}" %>[] =
{<%    key.mappings.each do |premods, mapping| %>
  { <%   if mapping.instance_of? Map
    %>MAP, <%
         elsif mapping.instance_of? Macro
    %>MACRO, <%
         elsif mapping.instance_of? Mode
    %>MODE, <%
         else
    %><%="/* What? */"%>, <%
         end
%><%= premods %>, (void*)&<%= mapping_identifier(keymap, key.location, premods, mapping.class) %> }, <%
       end %>
};
<%
     end
   end
%>


