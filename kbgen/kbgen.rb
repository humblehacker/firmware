#!/usr/bin/env ruby
#
#  Created by David Whetstone on 2007-03-24.
#  Copyright (c) 2007. All rights reserved.

require 'rubygems'
require 'set'
require 'hid_usages.rb'
require 'Keyboard.rb'
require 'KbParser.rb'
require 'optparse'
require 'erb'

require 'PP'

class ModeKey
  attr_accessor :name, :location, :matrix_cell, :type, :selectingMapID

  def initialize(name, location, matrix_cell, type, selectingMapID)
    @name = name
    @location = location
    @matrix_cell = matrix_cell
    @type = type
    @selectingMapID = selectingMapID
  end
end

def mapping_identifier(keymap, location, mods, mapping_type)
  "#{keymap.ids.last}_#{location}_#{mods}_#{mapping_type}"
end

def include_guard( filename, which )
  base = File.basename(filename, ".h").upcase
  extension = File.extname(filename).upcase
  guard = "__#{normalize_filename(base)}_#{extension.gsub(/\./, '')}__"
  case which
  when :begin
    return "#ifndef #{guard}\n#define #{guard}"
  when :end
    return "#endif /* #{guard} */"
  end
end

def delete_file(filename)
  if File.exists?(filename)
    puts "Deleting: #{filename}"
    File.delete(filename)
  end
end

def loc_to_hex(location)
  return '0x00' if location == nil
  return "0x#{location[1,5]}"
end

def dec_to_hex(dec)
  return "0x#{('%x'%dec).rjust(2,'0')}"
end

def init(filename, options)
  # Parse the HID Usage Tables
  $hid = HIDUsageTable.new('hid_usages.kspec', $includes)

  # Parse the keyboard file
  $kbp = KSpecParser.new
  inputfile = File.open(filename).read

  parseResult = $kbp.parse(inputfile)
  if (not parseResult)
    puts "#{filename} failure"
    puts " line  : #{$kbp.failure_line}"
    puts " column: #{$kbp.failure_column}"
    puts " reason: #{$kbp.failure_reason}¥"
  end

  $invertedMatrix = {}
  $allModeKeys = {}
  $keymapIDs = {}
  $macros = {}

  $keyboard = parseResult.content
  $keyboard.maps.each_value do |map|
    keymapIdentifier = normalize_filename("#{map.id}")
    $keymapIDs[map.id] = keymapIdentifier
  end

end

# Create header and source files for maps
def create_individual_matrix_map_sourcefiles
  macroIndex = 0
  matrix = $keyboard.matrix
  if !matrix.empty? && !matrix[0].empty?
    $keyboard.maps.each_value do |keymap|
      modeKeys = {}
      kbIdentifier = $keymapIDs[keymap.id]
      puts kbIdentifier

      rows = matrix.size
      cols = matrix[0].size

      if keymap.type == 'system'
        storage = "PROGMEM"
      else
        storage = "EEMEM"
      end

      flippedMatrix = []
      matrix.each_index do |irow|
        row = matrix[irow]
        row.each_index do |icol|
          if flippedMatrix[icol] == nil
            flippedMatrix[icol] = Array.new
          end
          flippedMatrix[icol][irow] = row[icol]
        end
      end

      generate_from_template("#{kbIdentifier}_mx.h", "KeyMap.erb.h", binding)
      generate_from_template("#{kbIdentifier}_mx.c", "KeyMap.erb.c", binding)
    end
  end
end

# Create "ModeKeys.h" - defines the mode key tables, one set for each map
def create_modekeys_header
  $keyboard.maps.each_value do |map|
    modeKeys = {}
    kbIdentifier = $keymapIDs[map.id]

    sortedKeys = map.keys.sort
    index = 0
    sortedKeys.each do |loc, key|
      locIndex = loc[1,5].to_i(16)
      while( index < locIndex )
        index += 1
      end

      if key.mode != ''
        modeKeys[loc] = key.mode
        $allModeKeys[locIndex] = ModeKey.new(key.mode, locIndex, $keyboard.reverseMatrix[loc],
                                             key.modeType, key.modeMapID)
        index += 1
        next
      end

      next if key.usage == nil

      index += 1
    end
  end


  if !$allModeKeys.empty?

    modeKeyNames = {}
    nextID = 0
    $allModeKeys.each do |location, mode_key|
      if !modeKeyNames.has_key? mode_key.name
        modeKeyNames[mode_key.name] = nextID
        nextID += 1
      end
    end

    output = File.new("#{$options[:outdir]}/ModeKeys.h", 'w+')

    output.puts include_guard(output.path, :begin)
    output.puts
    output.puts '#include "config.h"'
    output.puts '#include "matrix.h"'
    output.puts '#include "hid_usages.h"'
    output.puts
    output.puts "#define MOMENTARY 0    // mode only active when mode key is pressed"
    output.puts "#define TOGGLE    1    // mode key toggles mode on and off"
    output.puts
    output.puts "typedef struct"
    output.puts "{"
    output.puts "  Cell cell;                // location of mode key"
    output.puts "  uint8_t type;             // type of mode key (MOMENTARY, TOGGLE)"
    output.puts "  MatrixMap selecting_map;  // the map this mode key selects"
    output.puts "} ModeKey;"
    output.puts
    output.puts "extern const ModeKey modeKeys[];"
    output.puts
    output.puts "#define MODE_KEY_NONE 0xff"
    modeKeyNames.each do |key, id|
      output.puts "#define MODE_KEY_#{normalize_identifier(key)} #{id}"
    end
    output.puts
    output.puts "#define NUM_MODE_KEYS #{modeKeyNames.size}"
    output.puts
    output.puts include_guard(output.path, :end)

    # also create ModeKeys.c
    output = File.new("#{$options[:outdir]}/ModeKeys.c", 'w+')

    output.puts '#include "keymaps.h"'
    output.puts '#include "ModeKeys.h"'
    output.puts
    output.puts "const ModeKey modeKeys[] PROGMEM = "
    output.puts "{"

    first = true
    $allModeKeys.each do |location, modeKey|
      output.puts "," if !first
      first = false

      keyid = modeKeyNames[modeKey.name]
      output.print "  /* #{modeKey.name} @ t#{'%x'%location} */"        # comment
      output.print " { MATRIX_CELL(#{modeKey.matrix_cell.row},"
      output.print              "#{modeKey.matrix_cell.col}),"          # matrixCell (e.g. MATRIX_CELL(5,2))
      output.print " #{modeKey.type.upcase},"                           # type       (e.g. MOMENTARY)
      output.print " kbd_map_#{$keymapIDs[modeKey.selectingMapID]}_mx }"# selectingMap
    end
    output.puts
    output.puts "};"

  end
end

def generate_from_template(filename, template, bindings)
  block = <<-EOF
    input  = File.new("../kbgen/templates/#{template}")
    output = File.new("#{$options[:outdir]}/#{filename}", 'w+')
    puts "Generating: \#{output.path} from \#{input.path}"
    output.puts "/* DO NOT EDIT.  This is a GENERATED FILE. */"
    output.puts "/* Edit instead: \#{input.path} */"
    output.puts ERB.new(input.read,0,'<>').result(binding)
  EOF
  eval(block, bindings)
end

def generate(filename, bindings)
  erbfilename = filename.sub(/(\.[ch])$/, '.erb\1')
  generate_from_template(filename, erbfilename, bindings)
end

# Start here
$includes = Includes.new

$options = {}
opts = OptionParser.new do |opts|
  opts.on("-I [INCLUDE]") do |path|
    $includes.push path
  end
  opts.on("-d", "--debug", "enable debugging mode") do |debug|
    $options[:debug] = true
    puts "Debug mode enabled"
  end
  opts.on("-O [OUTDIR]") do |path|
    $options[:outdir] = path
  end
end

begin
  opts.parse!(ARGV)
rescue Exception => e
  puts e, "", opts
  exit 1
end

filename = opts.default_argv[0]

begin
  init(filename, $options)
  generate("hid_usages.h", binding)
  generate("matrix.h", binding)
  generate("mapping.h", binding)
  generate("mapping.c", binding)
  create_individual_matrix_map_sourcefiles
  generate("keymaps.h", binding)
  generate("keymaps.c", binding)
# create_modekeys_header
rescue Exception => e
  puts e.to_str
  raise
end
