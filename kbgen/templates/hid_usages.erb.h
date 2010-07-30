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

#ifndef __HIDUSAGETABLES_H__
#define __HIDUSAGETABLES_H__

/*
   The largest HID usage code is AC_Distribute_Vertically from the Consumer
   Control usage page.  Its id is 0x29c, which requires 10 bits.  The largest
   usage page used by a keyboard, on the other hand, is 0x0c, which requires
   only 4 bits.  So we pack the usage id in the lower 10 bits of a 16 bit int,
   and the usage page in the upper 6 bits.
*/

#define MAKE_USAGE(page,id) (((page)<<10)|(id))
#define USAGE_ID(usage)     (usage&~(0xffff<<10))
#define USAGE_PAGE(usage)   (usage>>10)

typedef uint8_t  UsagePage;
typedef uint8_t  UsageID;
typedef enum
{
  HID_USAGE_NONE = 0UL,

<% $hid.pages.each do |usagePage| %>
  /* USAGE PAGE:  <%=usagePage.name %> */
#define HID_USAGE_PAGE_<%= normalize_identifier(usagePage.name)%> <%=dec_to_hex(usagePage.id)%>

<%   $hid.usagesByPage[usagePage].each do |usage| %>
  HID_USAGE_<%=normalize_identifier(usage.name)%> = MAKE_USAGE(<%=dec_to_hex(usagePage.id)%>, <%=dec_to_hex(usage.id)%>),
<%   end %>
<% end %>

} Usage;

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
  A_CTL = (1<<8),
  A_SHF = (1<<9),
  A_ALT = (1<<10),
  A_GUI = (1<<11),
} Modifiers;

static inline
Modifiers
get_modifier(Usage usage)
{
  switch(usage)
  {
  case HID_USAGE_LEFT_CONTROL:
    return L_CTL;
  case HID_USAGE_LEFT_SHIFT:
    return L_SHF;
  case HID_USAGE_LEFT_ALT:
    return L_ALT;
  case HID_USAGE_LEFT_GUI:
    return L_GUI;
  case HID_USAGE_RIGHT_CONTROL:
    return R_CTL;
  case HID_USAGE_RIGHT_SHIFT:
    return R_SHF;
  case HID_USAGE_RIGHT_ALT:
    return R_ALT;
  case HID_USAGE_RIGHT_GUI:
    return R_GUI;
  default:
    return NONE;
  }
}

#endif /* __HIDUSAGETABLES_H__ */

