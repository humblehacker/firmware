#!/usr/bin/env ruby
#
#  Created by David Whetstone on 2007-03-28.
#  Copyright (c) 2007. All rights reserved.

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
  attr_accessor :keys, :ids, :revs, :types

  def initialize(id, rev, type)
    @ids   = [id]
    @revs  = [rev]
    @types = [type]
    @keys  = {}
  end
  
  def initialize_copy(other)
    @ids   = @ids.dup
    @revs  = @revs.dup
    @keys  = @keys.dup
    @types = @types.dup
  end

  def add_map(id, rev, type)
    @ids.push   id
    @revs.push  rev
    @types.push type
  end
  
  def id
    @ids.last
  end
  
  def rev
    @revs.last
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
  attr_accessor :location, :label, :usage, :macro, :prevKey, :color, 
                :mode, :modeMapID, :modeType, :modeLED
  def initialize(location, color)
    @location = location
    @label = KeyLabel.new
    @usage = nil
    @macro = []
    @prevKey = nil
    @color = color
    @mode = ''
    @modeMapID = ''
    @modeType = nil
    @modeLED = nil
  end
  
  def empty?
    @label.empty? && @usage == nil && @macro.empty?
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

Modifiers =     { :L_CTRL => (1<<0), :L_SHFT => (1<<1), :L_ALT => (1<<2), :L_GUI => (1<<3),
                  :R_CTRL => (1<<4), :R_SHFT => (1<<5), :R_ALT => (1<<6), :R_GUI => (1<<7) }
ModifierCodes = { :L_CTRL => 'LC',   :L_SHFT => 'LS',   :L_ALT => 'LA',   :L_GUI => 'LG',
                  :R_CTRL => 'RC',   :R_SHFT => 'RS',   :R_ALT => 'RA',   :R_GUI => 'RG' }

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

