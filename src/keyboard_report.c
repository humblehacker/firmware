#include "keyboard_report.h"

void
KeyboardReport__init(KeyboardReport *this)
{
  memset(this, 0, sizeof(KeyboardReport));
}

void
KeyboardReport__copy(KeyboardReport *this, USB_KeyboardReport_Data_t *dst)
{
  memcpy(dst, &this->report, sizeof(*dst));
}


void
KeyboardReport__add_key(KeyboardReport *this, Usage usage)
{
  this->report.KeyCode[this->num_keys] = USAGE_ID(usage);
  ++this->num_keys;
}

void
KeyboardReport__set_modifiers(KeyboardReport *this, uint8_t mod_mask)
{
  this->report.Modifier |= mod_mask;
}

void
KeyboardReport__reset_modifiers(KeyboardReport *this, uint8_t mod_mask)
{
  this->report.Modifier &= ~mod_mask;
}

uint8_t
KeyboardReport__get_modifiers(KeyboardReport *this)
{
  return this->report.Modifier;
}



