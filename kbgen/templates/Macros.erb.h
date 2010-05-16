#ifndef __MACROS_H__
#define __MACROS_H__

#include "HIDUsageTables.h"

typedef union
{
  uint8_t all;
  struct
  {
    uint8_t l_ctrl:1;
    uint8_t l_shft:1;
    uint8_t l_alt:1;
    uint8_t l_gui:1;
    uint8_t r_ctrl:1;
    uint8_t r_shft:1;
    uint8_t r_alt:1;
    uint8_t r_gui:1;
  };
} Modifiers;

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

