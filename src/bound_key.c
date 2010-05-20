#include <avr/pgmspace.h>
#include <stddef.h>
#include "bound_key.h"
#include "keyboard_class.h"

bool
BoundKey__is_active(BoundKey *key)
{
  return key->cell != DEACTIVATED;
}

void
BoundKey__deactivate(BoundKey *key)
{
  key->cell = DEACTIVATED;
}

void
BoundKey__update_binding(BoundKey *key, Modifiers mods, KeyMap keymap)
{
  key->binding = NULL;

  static const KeyBindingArray bindings;
  memcpy_P((void*)&bindings, &keymap[key->cell], sizeof(keymap[key->cell]));
  if (bindings.length != 0)
  {
    // find and return the binding that matches the specified modifier state.
    for (int i = 0; i < bindings.length; ++i)
    {
      if (bindings.data[i].premods == mods)
      {
        key->binding = &bindings.data[i];
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
      key->binding = &bindings.data[0];
      return;
    }
  }
}

