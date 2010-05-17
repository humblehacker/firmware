#ifndef __MAPPING_H__
#define __MAPPING_H__

#include "hid_usages.h"
#include "matrix.h"

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
  } bits;
} Modifiers;

typedef enum {MAP, MODE, MACRO} MappingKind;

typedef struct
{
  MappingKind kind;
  Modifiers premods;
} Mapping;

typedef struct
{
  Mapping super;
  enum {MOMENTARY, TOGGLE} type;
  Mapping *selecting_map;
} ModeMapping;

typedef struct
{
  Mapping super;
  Modifiers modifiers;
  Usage usage;
} MapMapping;

typedef struct
{
  Mapping super;
  const MapMapping *mappings;
} MacroMapping;

typedef const Mapping*** KeyMap;

<% $keyboard.maps.each_value do |keymap|
     keymap.keys.each do |location, key| %>
extern const Mapping *<%= "#{keymap.ids.last}_#{key.location}" %>[];
<%
     end
   end
%>


#endif // __MAPPING_H__
