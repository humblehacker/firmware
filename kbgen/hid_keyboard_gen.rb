#!/usr/bin/env ruby
#
#  Created by David Whetstone on 2007-03-24.
#  Copyright (c) 2007. All rights reserved.

require 'rubygems'
require 'set'
require 'HIDUsageTable.rb'
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

def generate_from_template(filename, template, bindings)
  block = <<-EOF
    input  = File.new("../kbgen/templates/#{template}")
    output = File.new("#{$options[:outdir]}/#{filename}", 'w+')
    puts "Generating: \#{output.path} from \#{input.path}"
    output.puts ERB.new(input.read,0,'<>').result(binding)
  EOF
  eval(block, bindings)
end

def generate(filename, bindings)
  erbfilename = filename.sub(/(\.[ch])$/, '.erb\1')
  generate_from_template(filename, erbfilename, bindings)
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
  $hid = HIDUsageTable.new('HIDUsageTables.kspec', $includes)

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

def create_macro_header
  $macros = {}
  output = File.new("#{$options[:outdir]}/Macros.h", 'w+')
  template = ERB.new("../kbgen/templates/Macros.erb.h")
  output.puts template.result(binding)

  # also write Macro.c
  output = File.new("#{$options[:outdir]}/Macros.c", 'w+')

  output.puts "#include \"config.h\""
  output.puts "#include \"Macros.h\""
  output.puts

  index = 0
  $keyboard.maps.each_value do |map|
    sortedKeys = map.keys.sort
    sortedKeys.each do |loc, key|
      next if key.macro.empty?

      output.puts
      output.puts "/* #{map.id} @ #{loc} */"
      output.puts "const Macro p_macro#{index} PROGMEM = "
      output.puts "{"
      output.puts "  #{key.macro.size},"
      output.puts "  {"
      index += 1
      key.macro.each do |macroKey|
#          puts macroKey.usage.page
        next if macroKey.usage.page.id != 0x07 # Keyboard and Keypad
        output.print "    { {#{dec_to_hex(macroKey.modifiers)}}, "
        output.print         "MAKE_USAGE(#{dec_to_hex(macroKey.usage.page.id)}, "
        output.puts          "#{dec_to_hex(macroKey.usage.id)})},"
      end
      output.puts "  }"
      output.puts "};"
    end
  end

  output.puts
  output.puts "const Macro * p_macros[#{index}] PROGMEM = "
  output.puts "{"

  index.times do |i|
    output.puts "  &p_macro#{i},"
  end

  output.puts "};"

end

# Create "keymaps.h" - includes all of the generated keymap headers
def create_keymaps_header
  output = File.new("#{$options[:outdir]}/keymaps.h", 'w+')

  output.puts include_guard(output.path, :begin)
  output.puts
  output.puts '#include "config.h"'
  output.puts
  $keymapIDs.each do |name, id|
    output.puts "#include \"#{id}_mx.h\""
  end
  output.puts

  output.puts include_guard(output.path, :end)

  output = File.new("#{$options[:outdir]}/keymaps.c", 'w+')

  output.puts '#include "config.h"'
  output.puts

  $keymapIDs.each do |name, id|
#   output.puts "#include \"#{id}.c\""
    output.puts "#include \"#{id}_mx.c\""
  end
  output.puts

end

# Create header and source files for maps
def create_individual_matrix_map_sourcefiles
  macroIndex = 0
  matrix = $keyboard.matrix
  if !matrix.empty? && !matrix[0].empty?
    $keyboard.maps.each_value do |map|
      modeKeys = {}
      kbIdentifier = $keymapIDs[map.id]
      puts kbIdentifier
#     header = File.new("#{$options[:outdir]}/#{kbIdentifier}_mx.h", 'w+')
#     source = File.new("#{$options[:outdir]}/#{kbIdentifier}_mx.c", 'w+')

      rows = matrix.size
      cols = matrix[0].size

      if map.type == 'system'
        storage = "PROGMEM"
      else
        storage = "EEMEM"
      end

      generate_from_template("#{kbIdentifier}_mx.h", "KeyMap.erb.h", binding)

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

#     source.puts "#include \"#{header.path}\""

      generate_from_template("#{kbIdentifier}_mx.c", "KeyMap.erb.c", binding)

#     source.puts '#include "HIDUsageTables.h"'
#     source.puts '#include "ModeKeys.h"'
#     source.puts
#     source.print "const uint16_t kbd_map_#{kbIdentifier}_mx[] #{storage} ="
#     source.puts
#     source.puts "{"
#     flippedMatrix.each_index do |iCol|
#       if iCol > 0
#         source.puts ","
#         source.puts
#       end
#       source.puts "\t/* col: #{iCol} */"
#       source.puts
#       row = flippedMatrix[iCol]
#       row.each_index do |iRow|
#         location = row[iRow]
#         source.puts "," if iRow != 0
#         source.print "\t"
#         source.print "/* row:#{iRow} loc = #{location} */ "
#         key = map.keys[location]
#         if key == nil
#           source.print "HID_USAGE_NONE"
#         elsif !key.macro.empty?
#           source.print "HID_USAGE_MACRO(#{macroIndex})"
#           macroIndex += 1
#         elsif key.mode != ''
#           source.print "MAKE_USAGE(HID_USAGE_PAGE_CUSTOM, MODE_KEY_#{normalize_identifier(key.mode)})"
#         elsif key.usage == nil
#           source.print "HID_USAGE_NONE"
#         else
#           source.print "HID_USAGE_#{normalize_identifier(key.usage.name)}"
#         end
#       end
#     end
#
#     source.puts
#     source.puts "};"

#     if $keyboard.defaultMap == map.id
#       source.puts
#       source.puts "const MatrixMap kbd_map_mx_default PROGMEM = kbd_map_#{kbIdentifier}_mx;"
#     end

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
    output.puts '#include "HIDUsageTables.h"'
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
  generate("matrix.h", binding)
  generate("HIDUsageTables.h", binding)
  create_individual_matrix_map_sourcefiles
  create_keymaps_header
  create_modekeys_header
  # create_macro_header
rescue Exception => e
  puts e.to_str
  raise
end
