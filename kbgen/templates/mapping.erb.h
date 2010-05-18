#ifndef __MAPPING_H__
#define __MAPPING_H__

#include "hid_usages.h"
#include "matrix.h"

typedef enum
{
  NONE  = 0,
  L_CTL = (1<<0),
  L_SHF = (1<<1),
  L_ALT = (1<<2),
  L_GUI = (1<<3),
  R_CTL = (1<<4),
  R_SHF = (1<<5),
  R_ALT = (1<<6),
  R_GUI = (1<<7),
} Modifiers;

typedef enum {MAP, MODE, MACRO} MappingKind;

typedef struct
{
  MappingKind kind;
  Modifiers premods;
} Mapping;

typedef const Mapping*** KeyMap;

typedef struct
{
  Mapping super;
  enum {MOMENTARY, TOGGLE} type;
  KeyMap mode_map;
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

<% $keyboard.maps.each_value do |keymap|
     keymap.keys.each do |location, key| %>
extern const Mapping *<%= "#{keymap.ids.last}_#{key.location}" %>[];<%
     end
   end
%>


#endif // __MAPPING_H__
