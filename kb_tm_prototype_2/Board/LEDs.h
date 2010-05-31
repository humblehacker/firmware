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
			/** LED mask for the first LED on the board. */
			#define LEDS_LED1        (1<<0)

			/** LED mask for the second LED on the board. */
			#define LEDS_LED2        (1<<1)

			/** LED mask for the third LED on the board. */
			#define LEDS_LED3        (1<<2)

			/** LED mask for the fourth LED on the board. */
			#define LEDS_LED4        (1<<3)

			/** LED mask for all the LEDs on the board. */
			#define LEDS_ALL_LEDS    (LEDS_LED1 | LEDS_LED2 | LEDS_LED3 | LEDS_LED4)

			/** LED mask for the none of the board LEDs */
			#define LEDS_NO_LEDS     0

      /** Keyboard specific LED mask names */
      #define LED_NUM          LEDS_LED1
      #define LED_CAPS         LEDS_LED2
      #define LED_SCRL         LEDS_LED3
      #define LED_DV           LEDS_LED4

		/* Inline Functions: */
		#if !defined(__DOXYGEN__)
			static inline void LEDs_Init(void)
			{
        // Keyboard LEDs are F0-F3
        DDRF  |= LEDS_ALL_LEDS;  // set pins as outputs
        PORTF |= LEDS_ALL_LEDS;  // set pins high to prevent sinking (lights off)
			}

			static inline void LEDs_TurnOnLEDs(const uint8_t LEDMask)
			{
				PORTF &= ~LEDMask;
			}

			static inline void LEDs_TurnOffLEDs(const uint8_t LEDMask)
			{
				PORTF |= LEDMask;
			}

			static inline void LEDs_SetAllLEDs(const uint8_t LEDMask)
			{
        PORTF = ((PORTF | LEDS_ALL_LEDS) & ~LEDMask);
			}

			static inline void LEDs_ChangeLEDs(const uint8_t LEDMask, const uint8_t ActiveMask)
			{
        PORTF = ((PORTF | LEDMask) & ~ActiveMask);
			}

			static inline void LEDs_ToggleLEDs(const uint8_t LEDMask)
			{
        PORTF = (PORTF ^ (LEDMask & LEDS_ALL_LEDS));
			}

			static inline uint8_t LEDs_GetLEDs(void) ATTR_WARN_UNUSED_RESULT;
			static inline uint8_t LEDs_GetLEDs(void)
			{
				return ~(PORTF & LEDS_ALL_LEDS);
			}
		#endif

	/* Disable C linkage for C++ Compilers: */
		#if defined(__cplusplus)
			}
		#endif

#endif
