#include <avr/pgmspace.h>
#include <stddef.h>
#include "bound_key.h"
#include "keyboard_class.h"

void
BoundKey__set_cell(BoundKey *this, Cell cell)
{
  this->cell = cell;
  this->binding = NULL;
}

bool
BoundKey__is_active(BoundKey *this)
{
  return this->cell != DEACTIVATED;
}

void
BoundKey__deactivate(BoundKey *this)
{
  this->cell = DEACTIVATED;
}

void
BoundKey__update_binding(BoundKey *this, Modifiers mods, KeyMap keymap)
{
  this->binding = NULL;

  static const KeyBindingArray bindings;
  memcpy_P((void*)&bindings, &keymap[this->cell], sizeof(keymap[this->cell]));
  if (bindings.length != 0)
  {
    // find and return the binding that matches the specified modifier state.
    for (int i = 0; i < bindings.length; ++i)
    {
      if (bindings.data[i].premods == mods)
      {
        this->binding = &bindings.data[i];
        return;
      }
    }

    // TODO: fuzzier matching on modifer keys.

    // if no match was found, return the default binding
    // TODO: the code generator must ensure that the
    // following assumption is correct, the first
    // binding will be the one and only binding with
    // premods == NONE.
    if (bindings.data[0].premods == NONE)
    {
      this->binding = &bindings.data[0];
      return;
    }
  }
}

