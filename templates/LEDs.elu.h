/*
                   The HumbleHacker Keyboard Project
                 Copyright © 2008-2010, David Whetstone
              david DOT whetstone AT humblehacker DOT com

  This file is a part of the HumbleHacker Keyboard Firmware project.

  The HumbleHacker Keyboard Project is free software: you can
  redistribute it and/or modify it under the terms of the GNU General
  Public License as published by the Free Software Foundation, either
  version 3 of the License, or (at your option) any later version.

  The HumbleHacker Keyboard Project is distributed in the
  hope that it will be useful, but WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with The HumbleHacker Keyboard Firmware project.  If not, see
  <http://www.gnu.org/licenses/>.

  --------------------------------------------------------------------

  This code is based on the LUFA library's LED driver stub, written
  by Dean Camera.

             LUFA Library
     Copyright (C) Dean Camera, 2010.

  dean [at] fourwalledcubicle [dot] com
      www.fourwalledcubicle.com
  --------------------------------------------------------------------
*/

#ifndef __LEDS_USER_H__
#define __LEDS_USER_H__

	/* Includes: */
		#include <avr/io.h>

		// TODO: Add any required includes here

/* Enable C linkage for C++ Compilers: */
		#if defined(__cplusplus)
			extern "C" {
		#endif

	/* Preprocessor Checks: */
		#if !defined(__INCLUDE_FROM_LEDS_H)
			#error Do not include this file directly. Include LUFA/Drivers/Board/LEDS.h instead.
		#endif

    /* Public Interface - May be used in end-application: */
      /* Macros: */

        /** LED masks. */
<%
   stdleds = {}
   otherleds = {}
   for i,led in ipairs(kb.leds) do
     if led.std == nil then
       otherleds[#otherleds+1] = led
     end
   end
%>
        enum LEDs {
          LED_NUM_LOCK = (1<<0),
          LED_CAPS_LOCK = (1<<1),
          LED_SCROLL_LOCK = (1<<2),
          LED_COMPOSE = (1<<3),
          LED_KANA = (1<<4),
<% for i,led in ipairs(otherleds) do %>
			 LED_<%=string.upper(led.name)%> = (1<<<%=i+4%>),
<% end %>
        };

<% for i=1,4 do
     if kb.leds[i] ~= nil then %>
        #define LEDS_LED<%=i%> LED_<%=string.upper(kb.leds[i].name)%>
<%   end
   end%>

			/** LED mask for all the LEDs on the board. */
			#define LEDS_ALL_LEDS    (0<%
   for i,led in ipairs(kb.leds) do
     %> | LED_<%=string.upper(led.name)%><%
   end%>)

			/** LED mask for the none of the board LEDs */
			#define LEDS_NO_LEDS     0

		/* Inline Functions: */
		#if !defined(__DOXYGEN__)
			static inline void LEDs_Init(void)
			{
 <% for i,led in ipairs(kb.leds) do
      port = string.upper(string.sub(led.pin,2,2))
      pin  = string.sub(led.pin,3,3)
      name = 'LED_' .. string.upper(led.name) %>
           // <%=name%>
           DDR<%=port%> |= (1<<<%=pin%>); // set pin as output
<%    if led.flow == 'sink' then %>
           PORT<%=port%> |= (1<<<%=pin%>); // set pin high to prevent sinking (led off)
<%    else %>
           PORT<%=port%> &= ~(1<<<%=pin%>); // set pin low (led off)
<%    end
    end %>
			}

			static inline void LEDs_TurnOnLEDs(const uint8_t LEDMask)
			{
<% for i,led in ipairs(kb.leds) do
      port = "PORT" .. string.upper(string.sub(led.pin,2,2))
      pin  = string.sub(led.pin,3,3)
      name = 'LED_' .. string.upper(led.name) %>
           if (LEDMask & <%=name%>)
<%    if led.flow == 'sink' then %>
             <%=port%> &= ~(1<<<%=pin%>);
<%    else %>
             <%=port%> |= (1<<<%=pin%>);
<%    end
    end %>
			}

			static inline void LEDs_TurnOffLEDs(const uint8_t LEDMask)
			{
<% for i,led in ipairs(kb.leds) do
      port = "PORT" .. string.upper(string.sub(led.pin,2,2))
      pin  = string.sub(led.pin,3,3)
      name = 'LED_' .. string.upper(led.name) %>
           if (LEDMask & <%=name%>)
<%    if led.flow == 'sink' then %>
             <%=port%> |= (1<<<%=pin%>);
<%    else %>
             <%=port%> &= ~(1<<<%=pin%>);
<%    end
    end %>
			}

			static inline void LEDs_SetAllLEDs(const uint8_t LEDMask)
			{
           LEDs_TurnOffLEDs(LEDS_ALL_LEDS);
           LEDs_TurnOnLEDs(LEDMask);
			}

			static inline void LEDs_ChangeLEDs(const uint8_t LEDMask, const uint8_t ActiveMask)
			{
           LEDs_TurnOffLEDs(LEDMask);
           LEDs_TurnOnLEDs(ActiveMask);
			}

			static inline void LEDs_ToggleLEDs(const uint8_t LEDMask)
			{
<% for i,led in ipairs(kb.leds) do
      port = "PORT" .. string.upper(string.sub(led.pin,2,2))
      pin  = string.sub(led.pin,3,3)
      name = 'LED_' .. string.upper(led.name) %>
           if (LEDMask & <%=name%>)
             <%=port%> ^= ~(1<<<%=pin%>);
<% end %>
			}

			static inline uint8_t LEDs_GetLEDs(void) ATTR_WARN_UNUSED_RESULT;
			static inline uint8_t LEDs_GetLEDs(void)
			{
           uint8_t result = 0;
<% for i,led in ipairs(kb.leds) do
      port = "PORT" .. string.upper(string.sub(led.pin,2,2))
      pin  = string.sub(led.pin,3,3)
      name = 'LED_' .. string.upper(led.name) %>
<%    if led.flow == 'sink' then %>
           if ((<%=port%> & ~(1<<<%=pin%>)) == <%=port%>)
<%    else %>
           if ((<%=port%> & (1<<<%=pin%>)) == <%=port%>)
<%    end %>
             result |= <%=name%>;
<%  end %>
           return result;
			}
		#endif

	/* Disable C linkage for C++ Compilers: */
		#if defined(__cplusplus)
			}
		#endif

#endif
