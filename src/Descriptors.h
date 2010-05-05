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
     Copyright (C) Dean Camera, 2010.
              
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
		#include <avr/pgmspace.h>

		#include <LUFA/Drivers/USB/USB.h>

		#include <LUFA/Drivers/USB/Class/HID.h>

	/* Type Defines: */
		/** Type define for the device configuration descriptor structure. This must be defined in the
		 *  application code, as the configuration descriptor contains several sub-descriptors which
		 *  vary between devices, and which describe the device's usage to the host.
		 */
		typedef struct
		{
			USB_Descriptor_Configuration_Header_t Config;
			USB_Descriptor_Interface_t            Interface;
			USB_HID_Descriptor_t                  KeyboardHID;
	        USB_Descriptor_Endpoint_t             KeyboardEndpoint;
		} USB_Descriptor_Configuration_t;
					
	/* Macros: */
		/** Endpoint number of the Keyboard HID reporting IN endpoint. */
		#define KEYBOARD_EPNUM               1
		
		/** Size in bytes of the Keyboard HID reporting IN and OUT endpoints. */		
		#define KEYBOARD_EPSIZE              8

	/* Function Prototypes: */
		uint16_t CALLBACK_USB_GetDescriptor(const uint16_t wValue, const uint8_t wIndex, void** const DescriptorAddress)
		                           ATTR_WARN_UNUSED_RESULT ATTR_NON_NULL_PTR_ARG(3);

#endif
