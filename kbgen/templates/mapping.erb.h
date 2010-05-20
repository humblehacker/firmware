/*
                    The HumbleHacker Keyboard Project
                 Copyright © 2008-2010, David Whetstone
               david DOT whetstone AT humblehacker DOT com

  This file is a part of The HumbleHacker Keyboard Project.

  The HumbleHacker Keyboard Project is free software: you can redistribute
  it and/or modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  The HumbleHacker Keyboard Project is distributed in the hope that it will
  be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
  Public License for more details.

  You should have received a copy of the GNU General Public License along
  with The HumbleHacker Keyboard Project.  If not, see
  <http://www.gnu.org/licenses/>.

*/

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
