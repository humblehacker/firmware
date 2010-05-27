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

#ifndef __BINDING_H__
#define __BINDING_H__

#include "hid_usages.h"
#include "matrix.h"

typedef struct
{
  enum {NOMAP, MAP, MODE, MACRO} kind;
  Modifiers premods;
  void *target;
} KeyBinding;

static inline
void
KeyBinding__get(KeyBinding *binding, const KeyBinding *from)
{
  memcpy_P((void*)binding, (PGM_VOID_P)from, sizeof(KeyBinding));
}

static inline
void
KeyBinding__copy(KeyBinding *dst, const KeyBinding *src)
{
  dst->kind = src->kind;
  dst->premods = src->premods;
  dst->target = src->target;
}

typedef struct
{
  uint8_t length;
  const KeyBinding *data;
} KeyBindingArray;

static inline
void
KeyBindingArray__get(KeyBindingArray *array, const KeyBindingArray *from)
{
  memcpy_P((void*)array, (PGM_VOID_P)from, sizeof(KeyBindingArray));
}

typedef const KeyBindingArray* KeyMap;

typedef struct
{
  enum {MOMENTARY, TOGGLE} type;
  KeyMap mode_map;
} ModeTarget;

static inline
void
ModeTarget__get(ModeTarget *target, const ModeTarget *from)
{
  memcpy_P((void*)target, (PGM_VOID_P)from, sizeof(ModeTarget));
}

typedef struct
{
  Modifiers modifiers;
  Usage usage;
} MapTarget;

static inline
void
MapTarget__get(MapTarget *target, const MapTarget *from)
{
  memcpy_P((void*)target, (PGM_VOID_P)from, sizeof(MapTarget));
}

typedef struct
{
  uint8_t length;
  const MapTarget *targets;
} MacroTarget;

static inline
void
MacroTarget__get(MacroTarget *target, const MacroTarget *from)
{
  memcpy_P((void*)target, (PGM_VOID_P)from, sizeof(MacroTarget));
}

<% $keyboard.maps.each_value do |keymap|
     keymap.keys.each do |location, key| %>
extern const KeyBinding <%= "#{keymap.ids.last}_#{key.location}" %>[] PROGMEM;<%
     end
   end
%>


#endif // __MAPPING_H__
