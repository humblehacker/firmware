#include <avr/pgmspace.h>
#include <stddef.h>
#include "bound_key.h"
#include "keyboard_class.h"

void
BoundKey__set_cell(BoundKey *this, Cell cell)
{
  this->cell = cell;
  this->binding.kind = NOMAP;
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
  this->binding.kind = NOMAP;

  static KeyBindingArray bindings;
  KeyBindingArray__get(&bindings, &keymap[this->cell]);
  if (bindings.length != 0)
  {
    // find the binding that matches the specified modifier state.
    for (int i = 0; i < bindings.length; ++i)
    {
      const KeyBinding *binding = KeyBindingArray__get_binding(&bindings, i);
      if (binding->premods == mods)
      {
        KeyBinding__copy(binding, &this->binding);
        return;
      }
    }

    // TODO: fuzzier matching on modifer keys.

    // if no match was found, use the default binding
    // TODO: the code generator must ensure that the
    // following assumption is correct, the first
    // binding will be the one and only binding with
    // premods == NONE.
    if (bindings.data[0].premods == NONE)
    {
      const KeyBinding *binding = KeyBindingArray__get_binding(&bindings, 0);
      KeyBinding__copy(binding, &this->binding);
      return;
    }
  }
}

