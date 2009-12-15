/* 
               The HumbleHacker Keyboard Firmware Project
                   Copyright © 2008, David Whetstone
              david DOT whetstone AT humblehacker DOT com
 
  This file is a part of the HumbleHacker Keyboard Firmware project.  
 
  The HumbleHacker Keyboard Firmware project is free software: you can 
  redistribute it and/or modify it under the terms of the GNU General 
  Public License as published by the Free Software Foundation, either 
  version 3 of the License, or (at your option) any later version.  

  The HumbleHacker Keyboard Firmware project is distributed in the 
  hope that it will be useful, but WITHOUT ANY WARRANTY; without even 
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
  PURPOSE.  See the GNU General Public License for more details.  

  You should have received a copy of the GNU General Public License 
  along with The HumbleHacker Keyboard Firmware project.  If not, see 
  <http://www.gnu.org/licenses/>.
 
  --------------------------------------------------------------------
 
  This code is based on the keyboard demonstration application by 
  Denver Gingerich.  
 
  Copyright 2008  Denver Gingerich (denver [at] ossguy [dot] com)
 
  --------------------------------------------------------------------
 
  Gingerich's keyboard demonstration application is based on the MyUSB 
  Mouse demonstration application, written by Dean Camera.  
 
             MyUSB Library
     Copyright (C) Dean Camera, 2008.
              
  dean [at] fourwalledcubicle [dot] com
      www.fourwalledcubicle.com
  --------------------------------------------------------------------
*/

/** \file
 *
 *  Header file for Descriptors.c.
 */

#ifndef _DESCRIPTORS_H_
#define _DESCRIPTORS_H_

	/* Includes: */
		#include <LUFA/Drivers/USB/USB.h>

		#include <avr/pgmspace.h>

	/* Type Defines: */
		/** Type define for the HID class specific HID descriptor, to describe the HID device's specifications. Refer to the HID
		 *  specification for details on the structure elements.
		 */	
		typedef struct
		{
			USB_Descriptor_Header_t Header;
				
			uint16_t                HIDSpec;
			uint8_t                 CountryCode;
		
			uint8_t                 TotalHIDReports;

			uint8_t                 HIDReportType;
			uint16_t                HIDReportLength;
		} USB_Descriptor_HID_t;
		
		/** Type define for the data type used to store HID report descriptor elements. */
		typedef uint8_t USB_Descriptor_HIDReport_Datatype_t;

		/** Type define for the device configuration descriptor structure. This must be defined in the
		 *  application code, as the configuration descriptor contains several sub-descriptors which
		 *  vary between devices, and which describe the device's usage to the host.
		 */
		typedef struct
		{
			USB_Descriptor_Configuration_Header_t Config;
			USB_Descriptor_Interface_t            Interface;
			USB_Descriptor_HID_t                  KeyboardHID;
	        USB_Descriptor_Endpoint_t             KeyboardEndpoint;
	        USB_Descriptor_Endpoint_t             KeyboardLEDsEndpoint;
		} USB_Descriptor_Configuration_t;
					
	/* Macros: */
		/** Endpoint number of the Keyboard HID reporting IN endpoint. */
		#define KEYBOARD_EPNUM               1

		/** Endpoint number of the Keyboard HID reporting OUT endpoint. */
		#define KEYBOARD_LEDS_EPNUM          2
		
		/** Size in bytes of the Keyboard HID reporting IN and OUT endpoints. */		
		#define KEYBOARD_EPSIZE              8

		/** Descriptor header type value, to indicate a HID class HID descriptor. */
		#define DTYPE_HID                 0x21
		
		/** Descriptor header type value, to indicate a HID class HID report descriptor. */
		#define DTYPE_Report              0x22

	/* Function Prototypes: */
		uint16_t USB_GetDescriptor(const uint16_t wValue, const uint8_t wIndex, void** const DescriptorAddress)
		                           ATTR_WARN_UNUSED_RESULT ATTR_NON_NULL_PTR_ARG(3);

#endif
