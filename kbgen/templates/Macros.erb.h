#ifndef __MACROS_H__
#define __MACROS_H__

#include "hid_usages.h"

typedef struct
{
  Modifiers mod;
  Usage usage;
} MacroKey;

typedef struct
{
  uint8_t num_keys;
  const MacroKey keys[];
} Macro;

<%
  index = 0
  $keyboard.maps.each_value do |map|
    sortedKeys = map.keys.sort
    sortedKeys.each do |loc, key|
      next if key.macro.empty?
%>
extern const Macro p_macro<%=index%>; /* <%=map.id%> @ <%=loc%> */"
<%
      index += 1
    end
  end
%>

extern const Macro * p_macros[<%=index%>];

#endif /* __MACROS_H__ */

