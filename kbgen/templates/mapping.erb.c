#include "mapping.h"

/* All Mappings */
<% $keyboard.maps.each_value do |keymap|
     keymap.keys.each do |location, key|
       key.mappings.each do |premods, mapping|
         ident = mapping_identifier(keymap, key.location, premods, mapping.class)
         if mapping.instance_of? Map %>
const MapMapping <%=ident%> = { {MAP, {<%=premods%>}}, {<%=mapping.modifiers%>}, MAKE_USAGE(<%=mapping.usage.page.id%>, <%=mapping.usage.id%>) };<%
         elsif mapping.instance_of? Macro %>
const MapMapping <%=ident%>Mappings[] =
{
<%         mapping.mappings.each do |macro_mapping| %>
  { {MAP, {0}}, {<%=macro_mapping.modifiers%>}, MAKE_USAGE(<%=macro_mapping.usage.page.id%>, <%=macro_mapping.usage.id%>) },
<%         end %>
};

const MacroMapping <%= ident %> = { {MACRO, {<%=premods%>}}, &<%=ident%>Mappings[0] }; <%
         elsif mapping.instance_of? Mode %>
const ModeMapping <%= ident %> = { }; <%
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
const Mapping *<%= "#{keymap.ids.last}_#{key.location}" %>[] =
{<%    key.mappings.each do |premods, mapping| %>
  (Mapping*)&<%= mapping_identifier(keymap, key.location, premods, mapping.class) %>,<%
       end %>
};
<%
     end
   end
%>


