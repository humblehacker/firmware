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

typedef struct ModeTarget ModeTarget;
typedef struct MacroTarget MacroTarget;
typedef struct MapTarget MapTarget;
typedef struct KeyBinding KeyBinding;
typedef struct KeyBindingArray KeyBindingArray;
typedef const KeyBindingArray* KeyMap;

/*
 *    KeyBinding
 */

struct KeyBinding
{
  enum {NOMAP, MAP, MODE, MACRO} kind;
  Modifiers premods;
  void *target;
};

void               KeyBinding__copy(const KeyBinding *this, KeyBinding *dst);
const ModeTarget*  KeyBinding__get_mode_target(const KeyBinding *this);
const MacroTarget* KeyBinding__get_macro_target(const KeyBinding *this);
const MapTarget*   KeyBinding__get_map_target(const KeyBinding *this);

/*
 *    KeyBindingArray
 */

struct KeyBindingArray
{
  uint8_t length;
  const KeyBinding *data;
} ;

const KeyBinding* KeyBindingArray__get_binding(const KeyBindingArray *this, uint8_t index);

static inline
void
KeyBindingArray__get(KeyBindingArray *array, const KeyBindingArray *from)
{
  memcpy_P((void*)array, (PGM_VOID_P)from, sizeof(KeyBindingArray));
}

/*
 *    ModeTarget
 */

struct ModeTarget
{
  enum {MOMENTARY, TOGGLE} type;
  KeyMap mode_map;
};

/*
 *    MapTarget
 */

struct MapTarget
{
  Modifiers modifiers;
  Usage usage;
};

/*
 *    MacroTarget
 */

struct MacroTarget
{
  uint8_t length;
  const MapTarget *targets;
};

const MapTarget* MacroTarget__get_map_target(const MacroTarget *this, uint8_t index);

/*
 *    Binding declarations
 */

<% $keyboard.maps.each_value do |keymap|
     keymap.keys.each do |location, key| %>
extern const KeyBinding <%= "#{keymap.ids.last}_#{key.location}" %>[] PROGMEM;<%
     end
   end
%>


#endif // __MAPPING_H__
