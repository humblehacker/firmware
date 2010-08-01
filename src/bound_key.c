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
BoundKey__update_binding(BoundKey *this, Modifier mods, KeyMap keymap)
{
  this->binding.kind = NOMAP;

  int best_index = 0;
  int best_rank  = 0;
  int current_rank;
  static KeyBindingArray bindings;
  KeyBindingArray__get(&bindings, &keymap[this->cell]);
  if (bindings.length != 0)
  {
    // find the binding that best matches the specified modifier state.
    for (int i = 0; i < bindings.length; ++i)
    {
      const KeyBinding *binding = KeyBindingArray__get_binding(&bindings, i);
      current_rank = PreMods__compare(&binding->premods, mods);
      if (current_rank > best_rank)
      {
        best_rank = current_rank;
        best_index = i;
      }
    }

    const KeyBinding *binding = KeyBindingArray__get_binding(&bindings, best_index);
    KeyBinding__copy(binding, &this->binding);
  }
}

