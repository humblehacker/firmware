#ifndef __KEYBOARDREPORT_H__
#define __KEYBOARDREPORT_H__

#include <LUFA/Drivers/USB/Class/HID.h>
#include "hid_usages.h"

struct KeyboardReport
{
  USB_KeyboardReport_Data_t report;
  uint8_t num_keys;
};

typedef struct KeyboardReport KeyboardReport;

void    KeyboardReport__init(KeyboardReport *this);
void    KeyboardReport__copy(KeyboardReport *this, USB_KeyboardReport_Data_t *dst);

void    KeyboardReport__add_key(KeyboardReport *this, Usage usage);
void    KeyboardReport__set_modifiers(KeyboardReport *this, uint8_t mod_mask);
void    KeyboardReport__reset_modifiers(KeyboardReport *this, uint8_t mod_mask);
uint8_t KeyboardReport__get_modifiers(KeyboardReport *this);

bool    KeyboardReport__has_key(KeyboardReport *this, Usage usage);


#endif // __KEYBOARDREPORT_H__
