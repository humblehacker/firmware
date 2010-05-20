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

  This code is based on the keyboard demonstration application by
  Denver Gingerich.

  Copyright 2008  Denver Gingerich (denver [at] ossguy [dot] com)

  --------------------------------------------------------------------

  Gingerich's keyboard demonstration application is based on the MyUSB
  Mouse demonstration application, written by Dean Camera.

             LUFA Library
     Copyright (C) Dean Camera, 2010.

  dean [at] fourwalledcubicle [dot] com
      www.fourwalledcubicle.com
  --------------------------------------------------------------------
*/

/** \file
 *
 *  Main source file for the Keyboard demo. This file contains the main tasks of the demo and
 *  is responsible for the initial application hardware configuration.
 */

#include <assert.h>
#include <util/delay.h>
#include "Keyboard.h"
#include "keymaps.h"
#include "mapping.c"
#include "keyboard_state.h"
#include "conf_keyboard.h"

/** Buffer to hold the previously generated Keyboard HID report, for comparison purposes inside the HID class driver. */
uint8_t PrevKeyboardHIDReportBuffer[sizeof(USB_KeyboardReport_Data_t)];

/** LUFA HID Class driver interface configuration and state information. This structure is
 *  passed to all HID Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_HID_Device_t Keyboard_HID_Interface =
 	{
		.Config =
			{
				.InterfaceNumber              = 0,

				.ReportINEndpointNumber       = KEYBOARD_EPNUM,
				.ReportINEndpointSize         = KEYBOARD_EPSIZE,
				.ReportINEndpointDoubleBank   = false,

				.PrevReportINBuffer           = PrevKeyboardHIDReportBuffer,
				.PrevReportINBufferSize       = sizeof(PrevKeyboardHIDReportBuffer),
			},
    };

uint8_t g_num_lock, g_caps_lock, g_scrl_lock;

/* Local Variables */
static   KeyMap s_current_kb_map;
static   KeyMap s_active_mode_kb_map;
static   KeyMap s_default_kb_map;

/* EEPROM Data */
//static KeyMap  EEMEM ee_persistent_map;

/* Local functions */
static              void scan_matrix(void);
static              bool momentary_mode_engaged(void);
static              bool modifier_keys_engaged(void);
static              void check_mode_toggle(void);
static              void process_keys(void);
static              void set_momentary_mode(KeyMap mode_map);
static         Modifiers get_modifier(Usage usage);
static const KeyMapping* get_mapping(Modifiers modifiers, Cell cell, KeyMap keymap);

/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{
	SetupHardware();

  LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);

	for (;;)
	{
		HID_Device_USBTask(&Keyboard_HID_Interface);
		USB_USBTask();
	}
}

/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardware()
{
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);

	/* Hardware Initialization */
	LEDs_Init();
	USB_Init();

  /* Task init */
  init_cols();
  keyboard_state__reset();

  g_num_lock           = g_caps_lock = g_scrl_lock = 0;
  s_active_mode_kb_map = NULL;
  s_current_kb_map = s_default_kb_map = (KeyMap) pgm_read_word(&kbd_map_mx_default);

#if defined(BOOTLOADER_TEST)
  uint8_t bootloader = eeprom_read_byte(&ee_bootloader);
  if (bootloader == 0xff) // eeprom has been reset
  {
    eeprom_write_byte(&ee_bootloader, FALSE);
  }
  else if (bootloader == TRUE)
  {
    asm("jmp 0xF000");
  }
#endif
}

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
  LEDs_SetAllLEDs(LEDMASK_USB_ENUMERATING);
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
  LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
  LEDs_SetAllLEDs(LEDMASK_USB_READY);

	if (!(HID_Device_ConfigureEndpoints(&Keyboard_HID_Interface)))
    ;
  LEDs_SetAllLEDs(LEDMASK_USB_ERROR);

	USB_Device_EnableSOFEvents();
}

/** Event handler for the library USB Unhandled Control Request event. */
void EVENT_USB_Device_UnhandledControlRequest(void)
{
	HID_Device_ProcessControlRequest(&Keyboard_HID_Interface);
}

/** Event handler for the USB device Start Of Frame event. */
void EVENT_USB_Device_StartOfFrame(void)
{
	HID_Device_MillisecondElapsed(&Keyboard_HID_Interface);
}

/** HID class driver callback function for the creation of HID reports to the host.
 *
 *  \param[in] HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in,out] ReportID  Report ID requested by the host if non-zero, otherwise callback should set to the generated report ID
 *  \param[in] ReportType  Type of the report to create, either REPORT_ITEM_TYPE_In or REPORT_ITEM_TYPE_Feature
 *  \param[out] ReportData  Pointer to a buffer where the created report should be stored
 *  \param[out] ReportSize  Number of bytes written in the report (or zero if no report is to be sent
 *
 *  \return Boolean true to force the sending of the report, false to let the library determine if it needs to be sent
 */
bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo, uint8_t* const ReportID,
                                         const uint8_t ReportType, void* ReportData, uint16_t* ReportSize)
{
  keyboard_state__reset();
  scan_matrix();
	keyboard_state__get_active_cells();

  if (!keyboard_state__is_error())
  {
    s_active_mode_kb_map = s_current_kb_map;
    g_kb_state.modifiers = 0;
    loop:
    if (momentary_mode_engaged())
      goto loop;
    if (modifier_keys_engaged())
      goto loop;
    check_mode_toggle();
    process_keys();
  }
	USB_KeyboardReport_Data_t* KeyboardReport = (USB_KeyboardReport_Data_t*)ReportData;

  *ReportSize = keyboard_state__fill_report(KeyboardReport);

	return false;
}

/** HID class driver callback function for the processing of HID reports from the host.
 *
 *  \param[in] HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in] ReportID  Report ID of the received report from the host
 *  \param[in] ReportData  Pointer to a buffer where the created report has been stored
 *  \param[in] ReportSize  Size in bytes of the received HID report
 */
void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo, const uint8_t ReportID,
                                          const void* ReportData, const uint16_t ReportSize)
{
  uint8_t* LEDReport = (uint8_t*)ReportData;

	g_num_lock  = (*LEDReport & (1<<0));
	g_caps_lock = (*LEDReport & (1<<1));
	g_scrl_lock = (*LEDReport & (1<<2));

  LEDs_ChangeLEDs(LED_CAPS|LED_SCRL|LED_NUM, *LEDReport);
}

static
void
scan_matrix()
{
  init_cols();
  for (uint8_t row = 0; row < NUM_ROWS; ++row)
  {
    activate_row(row);

    // Insert NOPs for synchronization
    _delay_us(20);

    // Place data on all column pins for active row into a single
    // 32 bit value.
		g_kb_state.row_data[row] = 0;
    g_kb_state.row_data[row] = read_row_data();
  }
}

static
const KeyMapping *
get_mapping(Modifiers modifiers, Cell cell, KeyMap keymap)
{
  static const KeyMappingArray mappings;
  memcpy_P((void*)&mappings, &keymap[cell], sizeof(keymap[cell]));
  if (mappings.length == 0)
    return NULL;

  // find and return the mapping that matches the specified modifier state.
  for (int i = 0; i < mappings.length; ++i)
  {
    if (mappings.data[i].premods == modifiers)
      return &mappings.data[i];
  }

  // TODO: fuzzier matching on modifer keys.

  // if no match was found, return the default mapping
  // TODO: the code generator must ensure that the
  // following assumption is correct, the first
  // mapping will be the one and only mapping with
  // premods == NONE.
  if (mappings.data[0].premods == NONE)
    return &mappings.data[0];

  // No matching mappings found
  return NULL;
}

static
void
set_momentary_mode(KeyMap mode_map)
{
  s_active_mode_kb_map = mode_map;
}

static
void
toggle_map(KeyMap mode_map)
{
  if (s_current_kb_map == mode_map)
    s_current_kb_map = s_default_kb_map;
  else
    s_current_kb_map = mode_map;
}

bool
momentary_mode_engaged()
{
  Cell active_cell;
  for (int i = 0; i < g_kb_state.num_active_cells; ++i)
  {
    active_cell = g_kb_state.active_cells[i];
    if (active_cell == DEACTIVATED)
      continue;
    const KeyMapping *mapping = get_mapping(g_kb_state.modifiers, active_cell, s_active_mode_kb_map);
    if (mapping->kind == MODE)
    {
      ModeTarget *target = (ModeTarget*)mapping->target;
      if (target->type == MOMENTARY)
      {
        set_momentary_mode(target->mode_map);
        g_kb_state.active_cells[i] = DEACTIVATED;
        return true;
      }
    }
  }
  return false;
}

bool
modifier_keys_engaged()
{
  Modifiers active_modifiers = NONE;
  Cell active_cell;
  for (int i = 0; i < g_kb_state.num_active_cells; ++i)
  {
    active_cell = g_kb_state.active_cells[i];
    if (active_cell == DEACTIVATED)
      continue;
    const KeyMapping *mapping = get_mapping(g_kb_state.modifiers, active_cell, s_active_mode_kb_map);
    if (mapping->kind == MAP)
    {
      const MapTarget *target = (const MapTarget*)mapping->target;
      Modifiers this_modifier = NONE;
      if ((this_modifier = get_modifier(target->usage)) != NONE)
      {
        active_modifiers |= this_modifier;
        g_kb_state.active_cells[i] = DEACTIVATED;
      }
    }
  }
  g_kb_state.modifiers |= active_modifiers;
  return active_modifiers != NONE;
}

static
void
check_mode_toggle(void)
{
  Cell active_cell;
  for (int i = 0; i < g_kb_state.num_active_cells; ++i)
  {
    active_cell = g_kb_state.active_cells[i];
    if (active_cell == DEACTIVATED)
      continue;
    const KeyMapping *mapping = get_mapping(g_kb_state.modifiers, active_cell, s_active_mode_kb_map);
    if (mapping->kind == MODE)
    {
      ModeTarget *target = (ModeTarget*)mapping->target;
      if (target->type == TOGGLE)
      {
        toggle_map(target->mode_map);
        g_kb_state.active_cells[i] = DEACTIVATED;
        return;
      }
    }
  }
}

static
void
process_keys()
{
  Cell active_cell;
  for (int i = 0; i < g_kb_state.num_active_cells; ++i)
  {
    active_cell = g_kb_state.active_cells[i];
    if (active_cell == DEACTIVATED)
      continue;
    const KeyMapping *mapping = get_mapping(g_kb_state.modifiers, active_cell, s_active_mode_kb_map);
    if (mapping->kind == MAP)
    {
      const MapTarget *target = (const MapTarget*)mapping->target;
      g_kb_state.keys[g_kb_state.num_keys] = target->usage;
      g_kb_state.modifiers &= ~mapping->premods;
      g_kb_state.modifiers |= target->modifiers;
      ++g_kb_state.num_keys;
    }
  }
}

static
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


