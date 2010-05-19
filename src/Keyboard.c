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
#include "Keyboard.h"
#include "keymaps.h"
#include "mapping.c"
#include "keyboard_state.h"
#include "kb_leds.h"

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

// TODO: Generate this value or get rid of it
#define NUM_MODE_KEYS 8

/* each row contains data for 3 Ports */
static KeyMap s_current_kb_map;
static KeyMap s_active_mode_kb_map;
static KeyMap s_default_kb_map;
//static KeyMap s_prev_kb_map[NUM_MODE_KEYS];
static uint32_t s_row_data[NUM_ROWS];             // Keep
static uint16_t s_timeout;
// static uint8_t s_idleDuration[3];

// -- EEPROM Data --
//static KeyMap  EEMEM ee_persistent_map;

/* Local functions */
static      void get_active_cells(void);
static      void scan_matrix(void);
static      bool momentary_mode_engaged(void);
static      bool modifier_keys_engaged(void);
static      void check_mode_toggle(void);
//static      void process_mode_keys(void);
static      void process_keys(void);
static      void fill_report(USB_KeyboardReport_Data_t* report);
static      void set_momentary_mode(KeyMap mode_map);
static Modifiers get_modifier(Usage usage);
// static     void process_consumer_control_endpoint(void);
static const KeyMapping* get_mapping(Modifiers modifiers, Cell cell, KeyMap keymap);

/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{
	SetupHardware();

//LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);

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
	USB_Init();

  /* Task init */
  init_cols();
  init_leds();
  keyboard_state__reset();

  g_num_lock           = g_caps_lock = g_scrl_lock = 0;
  s_active_mode_kb_map = NULL;
  s_current_kb_map = s_default_kb_map = (KeyMap) pgm_read_word(&kbd_map_mx_default);
  s_timeout            = 500;

  led_on(LED_NUM);

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
//LEDs_SetAllLEDs(LEDMASK_USB_ENUMERATING);
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
//LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
//LEDs_SetAllLEDs(LEDMASK_USB_READY);

	if (!(HID_Device_ConfigureEndpoints(&Keyboard_HID_Interface)))
    ;
//  LEDs_SetAllLEDs(LEDMASK_USB_ERROR);

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
	get_active_cells();

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
  fill_report(KeyboardReport);
  *ReportSize = sizeof(USB_KeyboardReport_Data_t);

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

	led_set(LED_CAPS, g_caps_lock);
	led_set(LED_SCRL, g_scrl_lock);
  led_set(LED_NUM,  g_num_lock);
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
    micro_pause(20);

    // Place data on all column pins for active row into a single
    // 32 bit value.
		s_row_data[row] = 0;
    s_row_data[row] = read_row_data();
  }
}

static
void
get_active_cells()
{
  uint8_t ncols;
  // now process row/column data to get raw keypresses
  for (uint8_t row = 0; row < NUM_ROWS; ++row)
  {
    ncols = 0;
    for (uint8_t col = 0; col < NUM_COLS; ++col)
    {
      if (s_row_data[row] & (1UL << col))
      {
        if (g_kb_state.num_active_cells > MAX_ACTIVE_CELLS)
        {
          g_kb_state.error_roll_over = TRUE;
          return;
        }
        ++ncols;
        g_kb_state.active_cells[g_kb_state.num_active_cells++] = MATRIX_CELL(row, col);
      }
    }

    // if 2 or more keys pressed in a row, check for ghost-key
    if (ncols > 1)
    {
      for (uint8_t irow = 0; irow < NUM_ROWS; ++irow)
      {
        if (irow == row)
          continue;

        // if any other row has a key pressed in the same column as any
        // of the two or more keys pressed in the current row, we have a
        // ghost-key condition.
        if (s_row_data[row] & s_row_data[irow])
        {
          g_kb_state.error_roll_over = TRUE;
          return;
        }
      }
    }
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
  for (int i = 0; i < mappings.length; ++i)
  {
    // return the mapping that matches the specified modifier state.
    if (mappings.data[i].premods == modifiers)
      return &mappings.data[i];
  }

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

#if 0
static
void
process_mode_keys()
{
  g_kb_state.mode_keys = 0;

  // First, check if any of the momentary-type mode keys are down
  ModeKey modeKey;
  Usage usage;
  uint8_t i, mode;
  for (i = 0; i < g_kb_state.num_active_cells; ++i)
  {
    for (mode = 0; mode < NUM_MODE_KEYS; ++mode)
    {
      memcpy_P(&modeKey,&(modeKeys[mode]),sizeof(ModeKey));
      if (modeKey.type != MOMENTARY)
        continue;

      usage = map_get_usage(s_current_kb_map, modeKey.cell);

      if (USAGE_PAGE(usage) == HID_USAGE_PAGE_CUSTOM
       && USAGE_ID(usage) == mode
       && g_kb_state.active_cells[i] == modeKey.cell)
      {
        g_kb_state.mode_keys |= (1<<mode);
        s_active_mode_kb_map = modeKey.selecting_map;
        // TODO: modeKeys[i].leds
        break;
      }
    }
  }

  // Now, process toggle-type mode keys
  for (i = 0; i < g_kb_state.num_active_cells; ++i)
  {
    for (mode = 0; mode < NUM_MODE_KEYS; ++mode)
    {
      memcpy_P(&modeKey,&(modeKeys[mode]),sizeof(ModeKey));
      if (modeKey.type != TOGGLE)
        continue;

      usage = map_get_usage(s_active_mode_kb_map, modeKey.cell);

      if (USAGE_PAGE(usage) == HID_USAGE_PAGE_CUSTOM
       && USAGE_ID(usage) == mode
       && g_kb_state.active_cells[i] == modeKey.cell)
      {
        g_kb_state.mode_keys |= (1<<mode);

        // if the current map is the same as the mode key's map,
        // set the current map back to the default, otherwise,
        // set the current map to the mode key's map.

        if (s_current_kb_map == modeKey.selecting_map)
        {
          s_current_kb_map = s_prev_kb_map[mode];
          eeprom_write_byte(&ee_persistent_map, MODE_KEY_NONE);
          // TODO: LEDs
        }
        else
        {
          s_prev_kb_map[mode] = s_current_kb_map;
          s_current_kb_map = modeKey.selecting_map;
          eeprom_write_byte(&ee_persistent_map, mode);
          // TODO: LEDs
        }
        break;
      }
    }
  }
}
#endif


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
void
fill_report(USB_KeyboardReport_Data_t* report)
{
  uint8_t key;

  if (keyboard_state__is_error())
  {
    report->Modifier = g_kb_state.modifiers;
    for (key = 1; key < 7; ++key)
      report->KeyCode[key] = USAGE_ID(HID_USAGE_ERRORROLLOVER);
    return;
  }

  if (!keyboard_state__is_processing_macro())
  {
    report->Modifier = g_kb_state.modifiers;

    for (key = 0; key < g_kb_state.num_keys; ++key)
      report->KeyCode[key] = g_kb_state.keys[key];
  }
  else
  {
    // TODO: Macro processing
#if 0
    const Macro * macro = g_kb_state.macro;
    MacroKey mkey;
    mkey.mod.all = pgm_read_byte(&macro->keys[g_kb_state.macro_key_index].mod);
    mkey.usage = pgm_read_word(&macro->keys[g_kb_state.macro_key_index].usage);
    uint8_t num_macro_keys = pgm_read_byte(&macro->num_keys);
    report->Modifier = g_kb_state.pre_macro_modifiers | mkey.mod.all;
    report->KeyCode[0] = USAGE_ID(mkey.usage);
    g_kb_state.macro_key_index++;
    if (g_kb_state.macro_key_index >= num_macro_keys)
    {
      g_kb_state.macro = NULL;
      g_kb_state.macro_key_index = 0;
    }
    return;
#endif
  }
}

static
Modifiers
get_modifier(Usage usage)
{
  switch(usage)
  {
  case HID_USAGE_LEFTCONTROL:
    return L_CTL;
  case HID_USAGE_LEFTSHIFT:
    return L_SHF;
  case HID_USAGE_LEFTALT:
    return L_ALT;
  case HID_USAGE_LEFT_GUI:
    return L_GUI;
  case HID_USAGE_RIGHTCONTROL:
    return R_CTL;
  case HID_USAGE_RIGHTSHIFT:
    return R_SHF;
  case HID_USAGE_RIGHTALT:
    return R_ALT;
  case HID_USAGE_RIGHT_GUI:
    return R_GUI;
  default:
    return NONE;
  }
}


