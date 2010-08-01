#!/usr/bin/env ruby
##
##                    The HumbleHacker Keyboard Project
##                 Copyright © 2008-2010, David Whetstone
##               david DOT whetstone AT humblehacker DOT com
##
##  This file is a part of The HumbleHacker Keyboard Project.
##
##  The HumbleHacker Keyboard Project is free software: you can redistribute
##  it and/or modify it under the terms of the GNU General Public License as
##  published by the Free Software Foundation, either version 3 of the
##  License, or (at your option) any later version.
##
##  The HumbleHacker Keyboard Project is distributed in the hope that it will
##  be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
##  Public License for more details.
##
##  You should have received a copy of the GNU General Public License along
##  with The HumbleHacker Keyboard Project.  If not, see
##  <http://www.gnu.org/licenses/>.

class KeyLayout
  attr_accessor :id, :spacing, :rows, :rev, :mount
  def initialize(id, rev)
    @id      = id
    @spacing = nil
    @mount   = nil
    @rows    = []
    @rev     = rev
  end
end

class KeyDef
  attr_accessor :id, :origin, :size, :null, :offset, :background, :bump
  def initialize(id=nil)
    @id = id
    @origin = Point.new(0.0, 0.0)
    @size   = Size.new(0.0, 0.0)
    @background = nil
    @null   = false
    @offset = Point.new(0.0, 0.0)
    @bump = false
  end
end

class RowDef
  attr_accessor :id, :offset, :keydefs
  def initialize
    @id      = ''
    @offset  = Point.new(0.0, 0.0)
    @keydefs = []
  end
end

class KeyMap
  attr_accessor :keys, :ids, :types

  def initialize(id, type='system')
    @ids   = [id]
    @types = [type]
    @keys  = {}
  end

  def initialize_copy(other)
    @ids   = @ids.dup
    @keys  = @keys.dup
    @types = @types.dup
  end

  def add_map(id, type='system')
    @ids.push   id
    @types.push type
  end

  def id
    @ids.last
  end

  def type
    @types.last
  end

end

class KeyLegend
  attr_accessor :text, :color, :note
  def initialize(text, color, note)
    @text = text
    @color = color
    @note = note
  end
end

class KeyLabel
  attr_reader :legends
  def initialize
    @legends = {}
  end

  def topLeft
    @legends[:topLeft]
  end

  def topLeft=(val)
    @legends[:topLeft] = val
  end

  def centerLeft
    @legends[:centerLeft]
  end

  def centerLeft=(val)
    @legends[:centerLeft] = val
  end

  def bottomLeft
    @legends[:bottomLeft]
  end

  def bottomLeft=(val)
    @legends[:bottomLeft] = val
  end

  def topCenter
    @legends[:topCenter]
  end

  def topCenter=(val)
    @legends[:topCenter] = val
  end

  def center
    @legends[:center]
  end

  def center=(val)
    @legends[:center] = val
  end

  def bottomCenter
    @legends[:bottomCenter]
  end

  def bottomCenter=(val)
    @legends[:bottomCenter] = val
  end

  def topRight
    @legends[:topRight]
  end

  def topRight=(val)
    @legends[:topRight] = val
  end

  def centerRight
    @legends[:centerRight]
  end

  def centerRight=(val)
    @legends[:centerRight] = val
  end

  def bottomRight
    @legends[:bottomRight]
  end

  def bottomRight=(val)
    @legends[:bottomRight] = val
  end

  def empty?
    @legends.empty?
  end
end

class Cell
  attr_accessor :row, :col
  def initialize(row, col)
    @row = row
    @col = col
  end
end

class Key
  attr_accessor :location, :kbindings
  def initialize(location, color='black')
    @location = location
    @kbindings = {}
  end

  def empty?
    @label.empty? && @usage == nil && @macro.empty? && @modifiers == 0
  end

  def overridden?
    @prevKey != nil
  end

  def usageChanged?
    key = self
    while key != nil
      if key.prevKey != nil
        if key.usage != nil && key.prevKey.usage !=nil
          if key.usage.id != key.prevKey.usage.id || key.usage.page != key.prevKey.usage.page
            return true
          end
        else
          return true
        end
      end
      key = key.prevKey
    end
    return false
  end
end

class Map
  attr_reader :usage, :modifiers
  def initialize(usage, modifiers)
    @usage = usage
    @modifiers = modifiers
  end
end

class Mode
  attr_reader :mode, :type
  attr_accessor :led
  def initialize(mode, type)
    @mode = mode
    @type = type
    @led  = ""
  end
end

class Macro
  attr_accessor :kbindings
  def initialize
    @kbindings = []
  end
end

Modifier =      { :L_CTL => (1<<0), :L_SHF => (1<<1),  :L_ALT => (1<<2),  :L_GUI => (1<<3),
                  :R_CTL => (1<<4), :R_SHF => (1<<5),  :R_ALT => (1<<6),  :R_GUI => (1<<7),
                  :A_CTL => (1<<8), :A_SHF => (1<<9),  :A_ALT => (1<<10), :A_GUI => (1<<11) }
ModifierCodes = { :L_CTL => 'LC',   :L_SHF => 'LS',    :L_ALT => 'LA',    :L_GUI => 'LG',
                  :R_CTL => 'RC',   :R_SHF => 'RS',    :R_ALT => 'RA',    :R_GUI => 'RG',
                  :A_CTL => 'AC',   :A_SHF => 'AS',    :A_ALT => 'AA',    :A_GUI => 'AG' }

# returns the low byte of the argument 'modifiers' as 'stdmods',
# and the high nibble of 'modifiers' duplicated as the high and low
# nibbles of 'anymods'
def get_premods(modifiers)
  stdmods = modifiers & 0x00FF
  anymods = (modifiers & 0x0F00) >> 4;
  anymods |= anymods >> 4;
  return stdmods, anymods
end

# returns the low byte of 'modifiers', with the 'anymods' portion
# shifted down to the position of the right modifiers.  For example,
# A_ALT would become R_ALT.
def get_mods(modifiers)
  return modifiers & 0x00FF | ((modifiers & 0x0F00) >> 4);
end

def modifier_symbol_from_name(mod_name, convertanymods=false)
  puts "mod_name: #{mod_name}"
  case mod_name.downcase
  when "left_alt"
    return :L_ALT
  when "left_control"
    return :L_CTL
  when "left_shift"
    return :L_SHF
  when "left_gui"
    return :L_GUI
  when "right_alt"
    return :R_ALT
  when "right_control"
    return :R_CTL
  when "right_shift"
    return :R_SHF
  when "right_gui"
    return :R_GUI
  when "alt", "control", "shift", "gui"
    if convertanymods
      case mod_name.downcase
        when "alt"
          return :L_ALT
        when "control"
          return :L_CTL
        when "shift"
          return :L_SHF
        when "gui"
          return :L_GUI
      end
    else
      case mod_name.downcase
        when "alt"
          return :A_ALT
        when "control"
          return :A_CTL
        when "shift"
          return :A_SHF
        when "gui"
          return :A_GUI
      end
    end
  when ""
    return 0
  else
    raise "Unknown modifier #{mod_text}"
  end
end

def process_modifier(mod_name, convertanymods=false)
  return Modifier[modifier_symbol_from_name(mod_name, convertanymods)]
end

class MacroKey
  attr_accessor :usage, :modifiers
  def initialize
    @usage = nil
    @modifiers = 0
  end
  def empty?
    return @usage == nil && @modifiers == 0
  end
end



# class KeyMount
#   attr_accessor :height, :width
#   @height = nil
#   @width = nil
# end

