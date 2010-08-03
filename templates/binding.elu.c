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

#include "binding.h"

/*
 *   PreMods
 */

                          /* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
static uint8_t bitcount[] = {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};

static inline
uint8_t hi_nibble(uint8_t val)
{
  return (val & 0xF0) >> 4;
}

static inline
uint8_t lo_nibble(uint8_t val)
{
  return val & 0x0F;
}

uint8_t
PreMods__compare(const PreMods *this, uint8_t mods)
{
  uint8_t count = 0;
  uint8_t lo_mods = lo_nibble(mods);
  uint8_t hi_mods = hi_nibble(mods);
  uint8_t lo_std  = lo_nibble(this->std);
  uint8_t hi_std  = hi_nibble(this->std);
  count += bitcount[lo_mods&lo_std];
  count += bitcount[hi_mods&hi_std];
  count += bitcount[((lo_mods&~lo_std)|(hi_mods&~hi_std))&lo_nibble(this->any)];
  return count;
}

bool
PreMods__is_empty(const PreMods *this)
{
  return this->std == NONE && this->any == NONE;
}

/*
 *    KeyBinding
 */

void
KeyBinding__copy(const KeyBinding *this, KeyBinding *dst)
{
  memcpy(dst, this, sizeof(KeyBinding));
}

const ModeTarget*
KeyBinding__get_mode_target(const KeyBinding *this)
{
  static ModeTarget target;
  memcpy_P((void*)&target, (PGM_VOID_P)this->target, sizeof(ModeTarget));
  return &target;
}

const MacroTarget*
KeyBinding__get_macro_target(const KeyBinding *this)
{
  static MacroTarget target;
  memcpy_P((void*)&target, (PGM_VOID_P)this->target, sizeof(MacroTarget));
  return &target;
}

const MapTarget*
KeyBinding__get_map_target(const KeyBinding *this)
{
  static MapTarget target;
  memcpy_P((void*)&target, (PGM_VOID_P)this->target, sizeof(MapTarget));
  return &target;
}

const ModifierTarget*
KeyBinding__get_modifier_target(const KeyBinding *this)
{
  static ModifierTarget target;
  memcpy_P((void*)&target, (PGM_VOID_P)this->target, sizeof(ModifierTarget));
  return &target;
}

/*
 *    KeyBindingArray
 */

const KeyBinding*
KeyBindingArray__get_binding(const KeyBindingArray *this, uint8_t index)
{
  static KeyBinding binding;
  static const KeyBinding *last_binding = NULL;
  if (&this->data[index] != last_binding)
  {
    memcpy_P((void*)&binding, (PGM_VOID_P)&this->data[index], sizeof(KeyBinding));
    last_binding = &this->data[index];
  }
  return &binding;
}

/*
 *    MacroTarget
 */

const MapTarget*
MacroTarget__get_map_target(const MacroTarget *this, uint8_t index)
{
  static MapTarget target;
  memcpy_P((void*)&target, (PGM_VOID_P)&this->targets[index], sizeof(MapTarget));
  return &target;
}

/*
 *    All Bindings
 */

<% for mapname,keymap in pairs(kb.keymaps) do
     for location,key in pairs(keymap.keys) do
       for i,binding in ipairs(key.bindings) do
         ident = binding_identifier(keymap, key.location, binding.premods, binding.class)
         if binding.class == 'Map' then
           if binding.usage.is_modifier then %>
const ModifierTarget <%=ident%> PROGMEM = { <%= modifier_symbol_from_name(binding.usage.name) %> };<%
           else
             mods = string.format("%03x", convert_anymods_to_stdmods(binding.modifiers)) %>
const MapTarget      <%=ident%> PROGMEM = { 0x<%= mods %>, HID_USAGE_<%= normalize_identifier(binding.usage.name) %> };<%
           end
         elseif binding.class == 'Macro' then %>
const MapTarget      <%=ident%>Targets[] PROGMEM =
{
<%         for i,map in ipairs(binding.maps) do
             mods = string.format("%02x", convert_anymods_to_stdmods(map.modifiers))
%>  { 0x<%= mods %>, HID_USAGE_<%= normalize_identifier(map.usage.name) %> },
<%         end %>
};

const MacroTarget    <%= ident %> PROGMEM = { <%= #binding.maps %>, &<%= ident %>Targets[0] }; <%
         elseif binding.class == 'Mode' then %>
const ModeTarget     <%= ident %> PROGMEM = { <%= string.upper(binding.type) %>, keymap_<%= binding.name %> }; <%
         else
           %><%="/* What? */"%><%
         end
       end
     end
   end
%>

/*
 *    Aggregated bindings per key
 */

<% for mapname,keymap in pairs(kb.keymaps) do
     for location,key in pairs(keymap.keys) do %>
const KeyBinding <%= keymap.name %>_<%= key.location %>[] PROGMEM =
{<%    for i,binding in ipairs(key.bindings) do %>
  { <%   if binding.class == 'Map' then
           if binding.usage.is_modifier then
    %>MODIFIER, <%
           else
    %>MAP, <%
           end
         elseif binding.class == 'Macro' then
    %>MACRO, <%
         elseif binding.class == 'Mode' then
    %>MODE, <%
         else
    %>/* What's a <%= binding.class %>? */ NULL, <%
         end
         stdmods = string.format("%02x", binding.premods.stdmods)
         anymods = string.format("%02x", binding.premods.anymods)
%>{0x<%=stdmods%>, 0x<%=anymods%>}, (void*)&<%= binding_identifier(keymap, key.location, binding.premods, binding.class) %> }, <%
       end %>
};
<%
     end
   end
%>


