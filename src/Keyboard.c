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
     Copyright (C) Dean Camera, 2009.
              
  dean [at] fourwalledcubicle [dot] com
      www.fourwalledcubicle.com
  --------------------------------------------------------------------
*/

/** \file
 *
 *  Main source file for the Keyboard demo. This file contains the main tasks of the demo and
 *  is responsible for the initial application hardware configuration.
 */
 
#include "Keyboard.h"
#include "keyboard_state.h"
#include "Macros.c"
#include "kb_leds.h"

typedef enum
{
  L_CTL = (1<<0),
  L_SHF = (1<<1),
  L_ALT = (1<<2),
  L_GUI = (1<<3),
  R_CTL = (1<<4),
  R_SHF = (1<<5),
  R_ALT = (1<<6),
  R_GUI = (1<<7),
} ModKey;

/* Project Tags, for reading out using the ButtLoad project */
BUTTLOADTAG(ProjName,     FIRMWARE_ID);
BUTTLOADTAG(BuildTime,    __TIME__);
BUTTLOADTAG(BuildDate,    __DATE__);
BUTTLOADTAG(LUFAVersion, "LUFA V" LUFA_VERSION_STRING);

/* Scheduler Task List */
TASK_LIST
{
	#if !defined(INTERRUPT_CONTROL_ENDPOINT)
	{ Task: USB_USBTask          , TaskStatus: TASK_STOP },
	#endif
	
	#if !defined(INTERRUPT_DATA_ENDPOINT)
	{ Task: USB_Keyboard_Report  , TaskStatus: TASK_STOP },
	#endif
};

/* Global Variables */
/** Indicates what report mode the host has requested, true for normal HID reporting mode, false for special boot
 *  protocol reporting mode.
 */
bool UsingReportProtocol = true;

/** Current Idle period. This is set by the host via a Set Idle HID class request to silence the device's reports
 *  for either the entire idle duration, or until the report status changes (e.g. the user moves the mouse).
 */
uint8_t IdleCount = 0;

/** Current Idle period remaining. When the IdleCount value is set, this tracks the remaining number of idle
 *  milliseconds. This is seperate to the IdleCount timer and is incremented and compared as the host may request 
 *  the current idle period via a Get Idle HID class request, thus its value must be preserved.
 */
uint16_t IdleMSRemaining = 0;

uint8_t g_num_lock, g_caps_lock, g_scrl_lock;

/* Local Variables */

/* each row contains data for 3 Ports */
static MatrixMap s_current_kb_map;
static MatrixMap s_temp_kb_map;
static MatrixMap s_prev_kb_map[NUM_MODE_KEYS];
static       uint32_t s_row_data[NUM_ROWS];
static       uint16_t s_timeout;
// static        uint8_t s_idleDuration[3];

// -- EEPROM Data --
static        uint8_t  EEMEM ee_persistent_map;

/* Local functions */
static     void get_keyboard_state(void);
static     void process_mode_keys(void);
static     void process_keys(void);
static     void send_keys(void);
static    Usage map_get_usage(MatrixMap map, uint8_t cell);
static   ModKey get_modifier(Usage usage);
// static     void process_consumer_control_endpoint(void);
static     void process_keyboard_endpoint(void);
static     void process_led_endpoint(void);

/** Main program entry point. This routine configures the hardware required by the application, then
 *  starts the scheduler to run the USB management task.
 */
int main(void)
{
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);

	/* Hardware Initialization */
// Joystick_Init();
// LEDs_Init();
	
	/* Millisecond timer initialization, with output compare interrupt enabled for the idle timing */
	OCR0A  = 0x7D;
	TCCR0A = (1 << WGM01);
	TCCR0B = ((1 << CS01) | (1 << CS00));
	TIMSK0 = (1 << OCIE0A);

	/* Indicate USB not ready */
	UpdateStatus(Status_USBNotReady);

	/* Initialize Scheduler so that it can be used */
	Scheduler_Init();

	/* Initialize USB Subsystem */
	USB_Init();

  /* Task init */
  init_cols();
  init_leds();
  keyboard_state__init();

  g_num_lock       = g_caps_lock = g_scrl_lock = 0;
  s_temp_kb_map    = NULL;
  s_current_kb_map = (MatrixMap) pgm_read_word(&kbd_map_mx_default);
  s_timeout        = 500;

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

	/* Scheduling - routine never returns, so put this last in the main function */
	Scheduler_Start();
}

/** Event handler for the USB_Connect event. This indicates that the device is enumerating via the status LEDs and
 *  starts the library USB task to begin the enumeration and USB management process.
 */
EVENT_HANDLER(USB_Connect)
{
	#if !defined(INTERRUPT_CONTROL_ENDPOINT)
	/* Start USB management task */
	Scheduler_SetTaskMode(USB_USBTask, TASK_RUN);
	#endif

	/* Indicate USB enumerating */
	UpdateStatus(Status_USBEnumerating);

	/* Default to report protocol on connect */
	UsingReportProtocol = true;
}

/** Event handler for the USB_Reset event. This fires when the USB interface is reset by the USB host, before the
 *  enumeration process begins, and enables the control endpoint interrupt so that control requests can be handled
 *  asynchronously when they arrive rather than when the control endpoint is polled manually.
 */
EVENT_HANDLER(USB_Reset)
{
	#if defined(INTERRUPT_CONTROL_ENDPOINT)
	/* Select the control endpoint */
	Endpoint_SelectEndpoint(ENDPOINT_CONTROLEP);

	/* Enable the endpoint SETUP interrupt ISR for the control endpoint */
	USB_INT_Enable(ENDPOINT_INT_SETUP);
	#endif
}

/** Event handler for the USB_Disconnect event. This indicates that the device is no longer connected to a host via
 *  the status LEDs.
 */
EVENT_HANDLER(USB_Disconnect)
{
	/* Stop running keyboard reporting and USB management tasks */
	#if !defined(INTERRUPT_DATA_ENDPOINT)
	Scheduler_SetTaskMode(USB_Keyboard_Report, TASK_STOP);
	#endif

	#if !defined(INTERRUPT_CONTROL_ENDPOINT)
	Scheduler_SetTaskMode(USB_USBTask, TASK_STOP);
	#endif
	
	/* Indicate USB not ready */
	UpdateStatus(Status_USBNotReady);
}

/** Event handler for the USB_ConfigurationChanged event. This is fired when the host sets the current configuration
 *  of the USB device after enumeration, and configures the keyboard device endpoints.
 */
EVENT_HANDLER(USB_ConfigurationChanged)
{
	/* Setup Keyboard Keycode Report Endpoint */
	Endpoint_ConfigureEndpoint(KEYBOARD_EPNUM, EP_TYPE_INTERRUPT,
		                       ENDPOINT_DIR_IN, KEYBOARD_EPSIZE,
	                           ENDPOINT_BANK_SINGLE);

	#if defined(INTERRUPT_DATA_ENDPOINT)
	/* Enable the endpoint IN interrupt ISR for the report endpoint */
	USB_INT_Enable(ENDPOINT_INT_IN);
	#endif

	/* Setup Keyboard LED Report Endpoint */
	Endpoint_ConfigureEndpoint(KEYBOARD_LEDS_EPNUM, EP_TYPE_INTERRUPT,
		                       ENDPOINT_DIR_OUT, KEYBOARD_EPSIZE,
	                           ENDPOINT_BANK_SINGLE);

	#if defined(INTERRUPT_DATA_ENDPOINT)
	/* Enable the endpoint OUT interrupt ISR for the LED report endpoint */
	USB_INT_Enable(ENDPOINT_INT_OUT);
	#endif

	/* Indicate USB connected and ready */
	UpdateStatus(Status_USBReady);

	#if !defined(INTERRUPT_DATA_ENDPOINT)
	/* Start running keyboard reporting task */
	Scheduler_SetTaskMode(USB_Keyboard_Report, TASK_RUN);
	#endif
}

/** Event handler for the USB_UnhandledControlPacket event. This is used to catch standard and class specific
 *  control requests that are not handled internally by the USB library (including the HID commands, which are
 *  all issued via the control endpoint), so that they can be handled appropriately for the application.
 */
EVENT_HANDLER(USB_UnhandledControlPacket)
{
	/* Handle HID Class specific requests */
	switch (bRequest)
	{
		case REQ_GetReport:
			if (bmRequestType == (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_INTERFACE))
			{
				USB_KeyboardReport_Data_t KeyboardReportData;

				/* Create the next keyboard report for transmission to the host */
				CreateKeyboardReport(&KeyboardReportData);

				/* Ignore report type and ID number value */
				Endpoint_Discard_Word();
				
				/* Ignore unused Interface number value */
				Endpoint_Discard_Word();

				/* Read in the number of bytes in the report to send to the host */
				uint16_t wLength = Endpoint_Read_Word_LE();
				
				/* If trying to send more bytes than exist to the host, clamp the value at the report size */
				if (wLength > sizeof(KeyboardReportData))
				  wLength = sizeof(KeyboardReportData);

				Endpoint_ClearSetupReceived();
	
				/* Write the report data to the control endpoint */
				Endpoint_Write_Control_Stream_LE(&KeyboardReportData, wLength);
				
				/* Finalize the stream transfer to send the last packet or clear the host abort */
				Endpoint_ClearSetupOUT();
			}
		
			break;
		case REQ_SetReport:
			if (bmRequestType == (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE))
			{
				Endpoint_ClearSetupReceived();
				
				/* Wait until the LED report has been sent by the host */
				while (!(Endpoint_IsSetupOUTReceived()));

				/* Read in the LED report from the host */
				uint8_t LEDStatus = Endpoint_Read_Byte();

				/* Process the incomming LED report */
				ProcessLEDReport(LEDStatus);
			
				/* Clear the endpoint data */
				Endpoint_ClearSetupOUT();

				/* Acknowledge status stage */
				while (!(Endpoint_IsSetupINReady()));
				Endpoint_ClearSetupIN();
			}
			
			break;
		case REQ_GetProtocol:
			if (bmRequestType == (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_INTERFACE))
			{
				Endpoint_ClearSetupReceived();
				
				/* Write the current protocol flag to the host */
				Endpoint_Write_Byte(UsingReportProtocol);
				
				/* Send the flag to the host */
				Endpoint_ClearSetupIN();

				/* Acknowledge status stage */
				while (!(Endpoint_IsSetupOUTReceived()));
				Endpoint_ClearSetupOUT();
			}
			
			break;
		case REQ_SetProtocol:
			if (bmRequestType == (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE))
			{
				/* Read in the wValue parameter containing the new protocol mode */
				uint16_t wValue = Endpoint_Read_Word_LE();
								
				Endpoint_ClearSetupReceived();

				/* Set or clear the flag depending on what the host indicates that the current Protocol should be */
				UsingReportProtocol = (wValue != 0x0000);

				/* Acknowledge status stage */
				while (!(Endpoint_IsSetupINReady()));
				Endpoint_ClearSetupIN();
			}
			
			break;
		case REQ_SetIdle:
			if (bmRequestType == (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE))
			{
				/* Read in the wValue parameter containing the idle period */
				uint16_t wValue = Endpoint_Read_Word_LE();
				
				Endpoint_ClearSetupReceived();
				
				/* Get idle period in MSB */
				IdleCount = (wValue >> 8);
				
				/* Acknowledge status stage */
				while (!(Endpoint_IsSetupINReady()));
				Endpoint_ClearSetupIN();
			}
			
			break;
		case REQ_GetIdle:
			if (bmRequestType == (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_INTERFACE))
			{		
				Endpoint_ClearSetupReceived();
				
				/* Write the current idle duration to the host */
				Endpoint_Write_Byte(IdleCount);
				
				/* Send the flag to the host */
				Endpoint_ClearSetupIN();

				/* Acknowledge status stage */
				while (!(Endpoint_IsSetupOUTReceived()));
				Endpoint_ClearSetupOUT();
			}

			break;
	}
}

/** ISR for the timer 0 compare vector. This ISR fires once each millisecond, and increments the
 *  scheduler elapsed idle period counter when the host has set an idle period.
 */
ISR(TIMER0_COMPA_vect, ISR_BLOCK)
{
	/* One millisecond has elapsed, decrement the idle time remaining counter if it has not already elapsed */
	if (IdleMSRemaining)
	  IdleMSRemaining--;
}

/** Fills the given HID report data structure with the next HID report to send to the host.
 *
 *  \param ReportData  Pointer to a HID report data structure to be filled
 */
void CreateKeyboardReport(USB_KeyboardReport_Data_t* ReportData)
{
	uint8_t JoyStatus_LCL = Joystick_GetStatus();

	/* Clear the report contents */
	memset(ReportData, 0, sizeof(USB_KeyboardReport_Data_t));

	if (JoyStatus_LCL & JOY_UP)
	  ReportData->KeyCode[0] = 0x04; // A
	else if (JoyStatus_LCL & JOY_DOWN)
	  ReportData->KeyCode[0] = 0x05; // B

	if (JoyStatus_LCL & JOY_LEFT)
	  ReportData->KeyCode[0] = 0x06; // C
	else if (JoyStatus_LCL & JOY_RIGHT)
	  ReportData->KeyCode[0] = 0x07; // D

	if (JoyStatus_LCL & JOY_PRESS)
	  ReportData->KeyCode[0] = 0x08; // E
}

/** Processes a received LED report, and updates the board LEDs states to match.
 *
 *  \param LEDReport  LED status report from the host
 */
void ProcessLEDReport(uint8_t LEDReport)
{
	g_num_lock  = (LEDReport & (1<<0));
	g_caps_lock = (LEDReport & (1<<1));
	g_scrl_lock = (LEDReport & (1<<2));

	led_set(LED_CAPS, g_caps_lock);
	led_set(LED_SCRL, g_scrl_lock);

#if 0
	uint8_t LEDMask = LEDS_LED2;
	
	if (LEDReport & 0x01) // NUM Lock
	  LEDMask |= LEDS_LED1;
	
	if (LEDReport & 0x02) // CAPS Lock
	  LEDMask |= LEDS_LED3;

	if (LEDReport & 0x04) // SCROLL Lock
	  LEDMask |= LEDS_LED4;

	/* Set the status LEDs to the current Keyboard LED status */
	LEDs_SetAllLEDs(LEDMask);
#endif
}

/** Sends the next HID report to the host, via the keyboard data endpoint. */
static inline void SendNextReport(void)
{
	static USB_KeyboardReport_Data_t PrevKeyboardReportData;
	USB_KeyboardReport_Data_t        KeyboardReportData;
	bool                             SendReport = true;
	
	/* Create the next keyboard report for transmission to the host */
	CreateKeyboardReport(&KeyboardReportData);
	
	/* Check if the idle period is set */
	if (IdleCount)
	{
		/* Check if idle period has elapsed */
		if (!(IdleMSRemaining))
		{
			/* Reset the idle time remaining counter, must multiply by 4 to get the duration in milliseconds */
			IdleMSRemaining = (IdleCount << 2);
		}
		else
		{
			/* Idle period not elapsed, indicate that a report must not be sent unless the report has changed */
			SendReport = (memcmp(&PrevKeyboardReportData, &KeyboardReportData, sizeof(USB_KeyboardReport_Data_t)) != 0);
		}
	}
	
	/* Save the current report data for later comparison to check for changes */
	PrevKeyboardReportData = KeyboardReportData;

	/* Select the Keyboard Report Endpoint */
	Endpoint_SelectEndpoint(KEYBOARD_EPNUM);

	/* Check if Keyboard Endpoint Ready for Read/Write, and if we should send a report */
	if (Endpoint_ReadWriteAllowed() && SendReport)
	{
		/* Write Keyboard Report Data */
		Endpoint_Write_Stream_LE(&KeyboardReportData, sizeof(KeyboardReportData));

		/* Finalize the stream transfer to send the last packet */
		Endpoint_ClearCurrentBank();
	}
}

/** Reads the next LED status report from the host from the LED data endpoint, if one has been sent. */
static inline void ReceiveNextReport(void)
{
	/* Select the Keyboard LED Report Endpoint */
	Endpoint_SelectEndpoint(KEYBOARD_LEDS_EPNUM);

	/* Check if Keyboard LED Endpoint Ready for Read/Write */
	if (!(Endpoint_ReadWriteAllowed()))
	  return;

	/* Read in the LED report from the host */
	uint8_t LEDReport = Endpoint_Read_Byte();

	/* Handshake the OUT Endpoint - clear endpoint and ready for next report */
	Endpoint_ClearCurrentBank();

	/* Process the read LED report from the host */
	ProcessLEDReport(LEDReport);
}

/** Function to manage status updates to the user. This is done via LEDs on the given board, if available, but may be changed to
 *  log to a serial port, or anything else that is suitable for status updates.
 *
 *  \param CurrentStatus  Current status of the system, from the Keyboard_StatusCodes_t enum
 */
void UpdateStatus(uint8_t CurrentStatus)
{
#if 0
	uint8_t LEDMask = LEDS_NO_LEDS;
	
	/* Set the LED mask to the appropriate LED mask based on the given status code */
	switch (CurrentStatus)
	{
		case Status_USBNotReady:
			LEDMask = (LEDS_LED1);
			break;
		case Status_USBEnumerating:
			LEDMask = (LEDS_LED1 | LEDS_LED2);
			break;
		case Status_USBReady:
			LEDMask = (LEDS_LED2 | LEDS_LED4);
			break;
	}
	
	/* Set the board LEDs to the new LED mask */
	LEDs_SetAllLEDs(LEDMask);
#endif
}

#if !defined(INTERRUPT_DATA_ENDPOINT)
/** Function to manage HID report generation and transmission to the host, when in report mode. */
TASK(USB_Keyboard_Report)
{
	/* Check if the USB system is connected to a host */
	if (USB_IsConnected)
	{
		process_keyboard_endpoint();

//  /* Send the next keypress report to the host */
//  SendNextReport();
		
		/* Process the LED report sent from the host */
		ReceiveNextReport();
	}
}
#endif

/** ISR for the general Pipe/Endpoint interrupt vector. This ISR fires when an endpoint's status changes (such as
 *  a packet has been received) on an endpoint with its corresponding ISR enabling bits set. This is used to send
 *  HID packets to the host each time the HID interrupt endpoints polling period elapses, as managed by the USB
 *  controller. It is also used to respond to standard and class specific requests send to the device on the control
 *  endpoint, by handing them off to the LUFA library when they are received.
 */
ISR(ENDPOINT_PIPE_vect, ISR_BLOCK)
{
	#if defined(INTERRUPT_CONTROL_ENDPOINT)
	/* Check if the control endpoint has received a request */
	if (Endpoint_HasEndpointInterrupted(ENDPOINT_CONTROLEP))
	{
		/* Clear the endpoint interrupt */
		Endpoint_ClearEndpointInterrupt(ENDPOINT_CONTROLEP);

		/* Process the control request */
		USB_USBTask();

		/* Handshake the endpoint setup interrupt - must be after the call to USB_USBTask() */
		USB_INT_Clear(ENDPOINT_INT_SETUP);
	}
	#endif

	#if defined(INTERRUPT_DATA_ENDPOINT)
	/* Check if keyboard endpoint has interrupted */
	if (Endpoint_HasEndpointInterrupted(KEYBOARD_EPNUM))
	{
		/* Select the Keyboard Report Endpoint */
		Endpoint_SelectEndpoint(KEYBOARD_EPNUM);

		/* Clear the endpoint IN interrupt flag */
		USB_INT_Clear(ENDPOINT_INT_IN);

		/* Clear the Keyboard Report endpoint interrupt */
		Endpoint_ClearEndpointInterrupt(KEYBOARD_EPNUM);

		/* Send the next keypress report to the host */
		SendNextReport();
	}

	/* Check if Keyboard LED status Endpoint has interrupted */
	if (Endpoint_HasEndpointInterrupted(KEYBOARD_LEDS_EPNUM))
	{
		/* Select the Keyboard LED Report Endpoint */
		Endpoint_SelectEndpoint(KEYBOARD_LEDS_EPNUM);

		/* Clear the endpoint OUT interrupt flag */
		USB_INT_Clear(ENDPOINT_INT_OUT);

		/* Clear the Keyboard LED Report endpoint interrupt */
		Endpoint_ClearEndpointInterrupt(KEYBOARD_LEDS_EPNUM);

		/* Process the LED report sent from the host */
		ReceiveNextReport();
	}
	#endif
}

static
void 
process_keyboard_endpoint(void)
{
	Endpoint_SelectEndpoint(KEYBOARD_EPNUM);
	if (!Endpoint_ReadWriteAllowed())
		return;

  get_keyboard_state();

  // check for changes
  uint8_t changed = keyboard_state__has_changed();

  // only send data if changed or timeout has occured
  if (changed == TRUE || s_timeout == 0)
  {
    s_timeout = 500; // milliseconds

    // prevents empty timeout reports from being sent.
    if (changed || !keyboard_state__is_empty())
    {
      if (!keyboard_state__is_error())
      {
        s_temp_kb_map = s_current_kb_map;
        process_mode_keys();
        process_keys();
      }
      send_keys();
    }

    // if processing macro, don't swap states until macro is complete
    if (changed == TRUE && keyboard_state__is_processing_macro() == FALSE )
      keyboard_state__swap_states();
  }
}

static
void 
process_led_endpoint(void)
{
///out///Endpoint_SelectEndpoint(KEYBOARD_LEDS_EPNUM);
///out///if (!Endpoint_ReadWriteAllowed())
///out///return;

  /* Read in the LED report from the host */
  uint8_t LEDStatus = Endpoint_Read_Byte();

  g_num_lock  = (LEDStatus & (1<<0));
  g_caps_lock = (LEDStatus & (1<<1));
  g_scrl_lock = (LEDStatus & (1<<2));

  led_set(LED_CAPS, g_caps_lock);
  led_set(LED_SCRL, g_scrl_lock);

  /* Handshake the OUT Endpoint - clear endpoint and ready for next report */
  Endpoint_ClearCurrentBank();
}

static
void
get_keyboard_state(void)
{
  keyboard_state__reset_current_state();

	uint8_t row, col;
  for (row = 0; row < NUM_ROWS; ++row)
  {
    // Activate the current row
    ROW_DDR  =  (1 << row);
    ROW_PORT = ~(1 << row);

    // Insert NOPs for synchronization
    micro_pause(20);

    // Place data on all column pins for active row into a single
    // 32 bit value.
		s_row_data[row] = 0;
    s_row_data[row] = compress_cols();
  }

  uint8_t irow, ncols;
  // now process row/column data to get raw keypresses
  for (row = 0; row < NUM_ROWS; ++row)
  {
    ncols = 0;
    for (col = 0; col < NUM_COLS; ++col)
    {
      if (s_row_data[row] & (1UL << col))
      {
        if (g_current_kb_state->num_active_cells > MAX_ACTIVE_CELLS)
        {
          g_current_kb_state->error_roll_over = TRUE;
          return;
        }
        ++ncols;
        g_current_kb_state->active_cells[g_current_kb_state->num_active_cells++] = MATRIX_CELL(row, col);
      }
    }

    // if 2 or more keys pressed in a row, check for ghost-key
    if (ncols > 1)
    {
      for (irow = 0; irow < NUM_ROWS; ++irow)
      {
        if (irow == row)
          continue;

        // if any other row has a key pressed in the same column as any
        // of the two or more keys pressed in the current row, we have a
        // ghost-key condition.
        if (s_row_data[row] & s_row_data[irow])
        {
          g_current_kb_state->error_roll_over = TRUE;
          return;
        }
      }
    }
  }
  return;
}

static
void
process_mode_keys()
{
  g_current_kb_state->mode_keys = 0;

  // First, check if any of the momentary-type mode keys are down
  ModeKey modeKey;
  Usage usage;
  uint8_t i, mode;
  for (i = 0; i < g_current_kb_state->num_active_cells; ++i)
  {
    for (mode = 0; mode < NUM_MODE_KEYS; ++mode)
    {
      memcpy_P(&modeKey,&(modeKeys[mode]),sizeof(ModeKey));
      if (modeKey.type != MOMENTARY)
        continue;

      usage = map_get_usage(s_current_kb_map, modeKey.cell);
  
      if (USAGE_PAGE(usage) == HID_USAGE_PAGE_CUSTOM
       && USAGE_ID(usage) == mode  
       && g_current_kb_state->active_cells[i] == modeKey.cell)
      {
        g_current_kb_state->mode_keys |= (1<<mode);
        s_temp_kb_map = modeKey.selecting_map;
        // TODO: modeKeys[i].leds
        break;
      }
    }                                                                           
  }

  // Now, process toggle-type mode keys
  for (i = 0; i < g_current_kb_state->num_active_cells; ++i)
  {
    for (mode = 0; mode < NUM_MODE_KEYS; ++mode)
    {
      memcpy_P(&modeKey,&(modeKeys[mode]),sizeof(ModeKey));
      if (modeKey.type != TOGGLE)
        continue;

      usage = map_get_usage(s_temp_kb_map, modeKey.cell);
  
      if (USAGE_PAGE(usage) == HID_USAGE_PAGE_CUSTOM
       && USAGE_ID(usage) == mode 
       && g_current_kb_state->active_cells[i] == modeKey.cell)
      {
        g_current_kb_state->mode_keys |= (1<<mode);

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


static
void
process_keys()
{
  MatrixMap map = s_temp_kb_map;
  uint8_t i, modifier;
  uint8_t num_blocked_keys = 0;
  Cell raw_key;
  Usage usage;
  UsagePage usage_page;
  for (i=0; i < g_current_kb_state->num_active_cells; ++i)
  {
    if (g_current_kb_state->num_keys > MAX_KEYS)
    {
      g_current_kb_state->error_roll_over = TRUE;
      break;
    }

    raw_key = g_current_kb_state->active_cells[i];

    if (keyboard_state__mode_keys_have_changed())
    {
      g_blocked_keys[num_blocked_keys++] = raw_key;
      continue;
    }

    usage = map_get_usage(map, raw_key);

    modifier = get_modifier(usage);
    if (modifier)
    {
      g_current_kb_state->modifiers |= modifier;
    }
    else
    {
      if (keyboard_state__key_is_blocked(raw_key))
        continue;

      usage_page = USAGE_PAGE(usage);

      if (usage_page == HID_USAGE_PAGE_KEYBOARD_AND_KEYPAD)
      {
        g_current_kb_state->keys[g_current_kb_state->num_keys] = USAGE_ID(usage);
        ++g_current_kb_state->num_keys;
        continue;
      }

      if (usage_page == HID_USAGE_PAGE_CONSUMER_CONTROL)
      {
        if (g_current_kb_state->consumer_key)
        {
          g_current_kb_state->error_roll_over = TRUE;
          break;
        }
        g_current_kb_state->consumer_key = USAGE_ID(usage);
        continue;
      }

      if (usage_page == HID_USAGE_PAGE_MACRO)
      {
        g_current_kb_state->macro = (const Macro *)pgm_read_word(&p_macros[USAGE_ID(usage)]);
        g_current_kb_state->pre_macro_modifiers = g_current_kb_state->modifiers;
        continue;
      }

      if (usage == HID_USAGE_CUSTOM)
        continue;
    }
  }
  if (keyboard_state__mode_keys_have_changed())
    g_num_blocked_keys = num_blocked_keys;
}

static
void
send_keys(void)
{
	Endpoint_SelectEndpoint(KEYBOARD_EPNUM);
	while(!Endpoint_ReadWriteAllowed());

  uint8_t key;

  if (keyboard_state__is_error())
  {
    Endpoint_Write_Byte(g_current_kb_state->modifiers); // Byte0: Modifiers still reported
    Endpoint_Write_Byte(0);                // Byte1: Reserved
    for (key = 1; key < 7; ++key)
      Endpoint_Write_Byte(USAGE_ID(HID_USAGE_KEYBOARD_ERRORROLLOVER));
		Endpoint_ClearCurrentBank();
    return;
  }

  if (!keyboard_state__is_processing_macro())
  {
    if (keyboard_state__cooked_keys_have_changed())
    {
      Endpoint_Write_Byte(g_current_kb_state->modifiers); // Byte0: Modifier
      Endpoint_Write_Byte(0);                // Byte1: Reserved

      for (key = 0; key < g_current_kb_state->num_keys; ++key)
      {
        Endpoint_Write_Byte(g_current_kb_state->keys[key]);
      }
      for (; key < 7; ++key)
      {
        Endpoint_Write_Byte(0);
      }
			Endpoint_ClearCurrentBank();
    }
  }
  else
  {
    const Macro * macro = g_current_kb_state->macro;
    MacroKey mkey;
    mkey.mod.all = pgm_read_byte(&macro->keys[g_current_kb_state->macro_key_index].mod);
    mkey.usage = pgm_read_word(&macro->keys[g_current_kb_state->macro_key_index].usage);
    uint8_t num_macro_keys = pgm_read_byte(&macro->num_keys);
    Endpoint_Write_Byte(g_current_kb_state->pre_macro_modifiers | mkey.mod.all);
    Endpoint_Write_Byte(0);
    Endpoint_Write_Byte(USAGE_ID(mkey.usage));
    for (key = 2; key < 7; ++key)
      Endpoint_Write_Byte(0);
		Endpoint_ClearCurrentBank();
    g_current_kb_state->macro_key_index++;
    if (g_current_kb_state->macro_key_index >= num_macro_keys)
    {
      g_current_kb_state->macro = NULL;
      g_current_kb_state->macro_key_index = 0;
    }
    return;
  }

#ifdef LEDS_DONE
  if (g_current_kb_state->num_keys)
    led_on(LED_GRN_1);
  else
    led_off(LED_GRN_1);
#endif
}

static
inline 
Usage 
map_get_usage(MatrixMap map, uint8_t cell)
{
  return pgm_read_word(map + cell);
}

static
ModKey
get_modifier(Usage usage)
{
  switch(usage)
  {
  case HID_USAGE_KEYBOARD_LEFTCONTROL:
    return L_CTL;
  case HID_USAGE_KEYBOARD_LEFTSHIFT:
    return L_SHF;
  case HID_USAGE_KEYBOARD_LEFTALT:
    return L_ALT;
  case HID_USAGE_KEYBOARD_LEFT_GUI:
    return L_GUI;
  case HID_USAGE_KEYBOARD_RIGHTCONTROL:
    return R_CTL;
  case HID_USAGE_KEYBOARD_RIGHTSHIFT:
    return R_SHF;
  case HID_USAGE_KEYBOARD_RIGHTALT:
    return R_ALT;
  case HID_USAGE_KEYBOARD_RIGHT_GUI:
    return R_GUI;
  default:
    return 0;
  }
}


