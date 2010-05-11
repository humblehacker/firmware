#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <avr/io.h>
#include <avr/wdt.h>

#include "Descriptors.h"

#include <LUFA/Version.h>                    // Library Version Information
#include <LUFA/Drivers/USB/USB.h>            // USB Functionality
#include <LUFA/Drivers/Board/Joystick.h>     // Joystick driver
#include <LUFA/Drivers/Board/LEDs.h>         // LEDs driver
#include <LUFA/Scheduler/Scheduler.h>        // Simple scheduler for task management

typedef uint8_t  U8 ;
typedef uint16_t U16;
typedef uint32_t U32;

#define FALSE   (0==1)
#define TRUE    (1==1)

#include "conf_keyboard.h"

#endif // __CONFIG_H__
