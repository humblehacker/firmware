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

typedef struct
{
  enum {NOMAP, MAP, MODE, MACRO} kind;
  Modifiers premods;
  void *target;
} KeyMapping;

typedef struct
{
  uint8_t length;
  const KeyMapping *data;
} KeyMappingArray;

typedef const KeyMappingArray* KeyMap;

typedef struct
{
  enum {MOMENTARY, TOGGLE} type;
  KeyMap mode_map;
} ModeTarget;

typedef struct
{
  Modifiers modifiers;
  Usage usage;
} MapTarget;

typedef struct
{
  uint8_t length;
  const MapTarget *targets;
} MacroTarget;

<% $keyboard.maps.each_value do |keymap|
     keymap.keys.each do |location, key| %>
extern const KeyMapping <%= "#{keymap.ids.last}_#{key.location}" %>[];<%
     end
   end
%>


#endif // __MAPPING_H__
