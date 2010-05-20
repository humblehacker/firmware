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


class Point
  attr_accessor :x, :y
  def initialize(x, y)
    @x = x
    @y = y
  end

  def offset(point)
    @x += point.x
    @y += point.y
    return self
  end

  def + (point)
    return Point.new(point.x + @x, point.y + @y)
  end

  def to_s
    "(#{@x}, #{@y})"
  end

  def to_a
    [@x, @y]
  end
end

class Size
  attr_accessor :width, :height
  def initialize(h, w)
    @width = w
    @height = h
  end

  def to_s
    "(#{@width}, #{@height})"
  end

  def to_a
    [@width, @height]
  end
end

def max?(a, b)
  a > b ? a : b
end

class Includes
  def initialize
    @includes = []
  end

  def push(path)
    @includes.push path.strip
  end

  def find_file(filename)
    newname = nil
    @includes.each do |inc|
      newname = "#{inc}/#{filename}"
      return newname if File.exist?(newname)
    end
    return nil
  end
end

def normalize_number( number )
  return number.to_i(16) if number =~ /^0x/
  return number.to_i
end

def normalize_identifier(identifier)
  normalized = identifier.upcase
# normalized.gsub!(/_/,  '_UNDERSCORE')
  normalized.gsub!(/!/,  '_EXCLAMATION')
  normalized.gsub!(/\?/, '_QUESTION')
  normalized.gsub!(/@/,  '_AT')
  normalized.gsub!(/#/,  '_POUND')
  normalized.gsub!(/\$/, '_DOLLAR')
  normalized.gsub!(/%/,  '_PERCENT')
  normalized.gsub!(/\^/, '_CARET')
  normalized.gsub!(/&/,  '_AMPERSAND')
  normalized.gsub!(/\*/, '_ASTERISK')
  normalized.gsub!(/\./, '_PERIOD')
  normalized.gsub!(/\,/, '_COMMA')
  normalized.gsub!(/;/,  '_SEMICOLON')
  normalized.gsub!(/:/,  '_COLON')
  normalized.gsub!(/=/,  '_EQUALS')
  normalized.gsub!(/\+/, '_PLUS')
  normalized.gsub!(/\-/, '_MINUS')
  normalized.gsub!(/\(/, '_LPAREN')
  normalized.gsub!(/\)/, '_RPAREN')
  normalized.gsub!(/\[/, '_LSQUAREBRACKET')
  normalized.gsub!(/\]/, '_RSQUAREBRACKET')
  normalized.gsub!(/\{/, '_LCURLYBRACE')
  normalized.gsub!(/\}/, '_RCURLYBRACE')
  normalized.gsub!(/>/,  '_GREATERTHAN')
  normalized.gsub!(/</,  '_LESSTHAN')
  normalized.gsub!(/\\/, '_BACKSLASH')
  normalized.gsub!(/\//, '_SLASH')
  normalized.gsub!(/\~/, '_TILDE')
  normalized.gsub!(/\|/, '_PIPE')
  normalized.gsub!(/`/,  '_BACKTICK')
  normalized.gsub!(/'/,  '_APOSTROPHE')
  normalized.gsub!(/"/,  '_QUOTE')
  normalized.gsub!(' ',  '_')
  return normalized
end

def normalize_filename(filename)
  normalized = filename.gsub(/-/, '_')
  return normalized
end

def unknown_element_error(element, state)
  error = "Parse error: Unknown element '#{element}' for [ "
  state.each {|x| error << ':' << x.to_s << ' ' }
  error << ']'
  return error
end

def unknown_modifier_error(modifier)
  error = "Parse error: Unknown modifier '#{modifier}'"
  return error
end

class ParseError < Exception
  def initialize(filenames)
    @filename = nil
    @filenames = filenames.clone
  end

  def to_str
    "\n\twhile parsing #{@filenames.last}"
  end
end

class UnknownElementError < ParseError
  def initialize(element, state, attr_hash, filenames)
    super(filenames)
    @element = element
    @state = state
    @attr_hash = attr_hash
  end

  def to_str
    msg = "Parse error: Unknown element '#{@element}' for [ "
    @state.each {|x| msg << ':' << x.to_s << ' ' }
    msg << ']'
    msg << "\n\twith attributes: #{@attr_hash}"
    msg << super
    return msg
  end
end

class UnknownModifierError < ParseError
  def initialize(modifier, filenames)
    super(filenames)
    @modifier = modifier
  end

  def to_str
    msg = "Parse error: Unknown modifier '#{@modifier}'"
    msg << super
  end
end
