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
  $hid = HIDUsageTable.new('HIDUsageTables.xml', $includes)

  # Parse the keyboard file
  $kbp = KbParser.new($hid, $includes, options, filename)

  $invertedMatrix = {}
  $allModeKeys = {}
  $keymapIDs = {}
  $macros = {}

  $kbp.keyboards.each do |keyboard|
    keyboard.maps.each do |map|
      keymapIdentifier = normalize_filename("#{keyboard.layout.id}_#{map.id}")
      $keymapIDs[map.id] = keymapIdentifier
    end
  end

end

# Create HIDUsageTables.h
def create_hid_usage_tables_header
  output = File.new("#{$options[:outdir]}/HIDUsageTables.h", 'w+')

  if $options[:clean]
    output.close
    delete_file(output.path)
    return
  end

  output.puts include_guard(output.path, :begin)
  output.puts
  output.puts "#define MAKE_USAGE(page,id) (((page)<<10)|(id))"
  output.puts "#define USAGE_ID(usage)     (usage&~(0xffff<<10))"
  output.puts "#define USAGE_PAGE(usage)   (usage>>10)"
  output.puts
  output.puts "#define HID_USAGE_NONE        0UL"
  output.puts "#define HID_USAGE_PAGE_CUSTOM 0x3f"
  output.puts "#define HID_USAGE_CUSTOM      MAKE_USAGE(HID_USAGE_PAGE_CUSTOM,0)"
  output.puts "#define HID_USAGE_PAGE_MACRO  0x3e"
  output.puts "#define HID_USAGE_MACRO(x)    MAKE_USAGE(HID_USAGE_PAGE_MACRO,(x))"
  output.puts
#  output.puts "typedef uint16_t Usage;"
  output.puts "typedef uint8_t  UsagePage;"
  output.puts "typedef uint8_t  UsageID;"

  output.puts "typedef enum"
  output.puts "{"

  $hid.usagePages.each do |usagePage|
    output.puts
    output.puts "/* USAGE PAGE:  #{usagePage.name} */"
    output.puts "#define HID_USAGE_PAGE_#{normalize_identifier(usagePage.name)} #{dec_to_hex(usagePage.id)}"
    output.puts
    usagePage.usages.each do |usage|
#      output.puts "#define HID_USAGE_#{normalize_identifier(usage.name)} MAKE_USAGE(#{dec_to_hex(usagePage.id)}, #{dec_to_hex(usage.id)})"
      output.puts "  HID_USAGE_#{normalize_identifier(usage.name)} = MAKE_USAGE(#{dec_to_hex(usagePage.id)}, #{dec_to_hex(usage.id)}),"
    end
  end

  output.puts "} Usage;"

  output.puts
  output.puts include_guard(output.path, :end)
end

def create_macro_header
  if $options[:clean]
    delete_file("#{$options[:outdir]}/Macros.h")
    delete_file("#{$options[:outdir]}/Macros.c")
    return
  end

  $macros = {}
  output = File.new("#{$options[:outdir]}/Macros.h", 'w+')

  output.puts include_guard(output.path, :begin)
  output.puts 
  output.puts '#include "HIDUsageTables.h"'
  output.puts
  output.puts "typedef union"
  output.puts "{"
  output.puts "  uint8_t all;"
  output.puts "  struct"
  output.puts "  {"
  output.puts "    uint8_t l_ctrl:1;"
  output.puts "    uint8_t l_shft:1;"
  output.puts "    uint8_t l_alt:1;"
  output.puts "    uint8_t l_gui:1;"
  output.puts "    uint8_t r_ctrl:1;"
  output.puts "    uint8_t r_shft:1;"
  output.puts "    uint8_t r_alt:1;"
  output.puts "    uint8_t r_gui:1;"
  output.puts "  };"
  output.puts "} Modifiers;"
  output.puts
  output.puts "typedef struct"
  output.puts "{"
  output.puts "  Modifiers mod;"
  output.puts "  Usage usage;"
  output.puts "} MacroKey;"
  output.puts
  output.puts "typedef struct"
  output.puts "{"
  output.puts "  uint8_t num_keys;"
  output.puts "  const MacroKey keys[];"
  output.puts "} Macro;"
  output.puts

  index = 0
  $kbp.keyboards.each do |keyboard|
    keyboard.maps.each do |map|
      sortedKeys = map.keys.sort
      sortedKeys.each do |loc, key|
        next if key.macro.empty?

        output.puts "extern const Macro p_macro#{index}; /* #{map.id} @ #{loc} */"
        index += 1
      end
    end
  end

  output.puts
  output.puts "extern const Macro * p_macros[#{index}];";
  output.puts
  output.puts include_guard(output.path, :end)

  # also write Macro.c
  output = File.new("#{$options[:outdir]}/Macros.c", 'w+')

  output.puts "#include \"config.h\""
  output.puts "#include \"Macros.h\""
  output.puts

  index = 0
  $kbp.keyboards.each do |keyboard|
    keyboard.maps.each do |map|
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
  end

  output.puts
  output.puts "const Macro * p_macros[#{index}] PROGMEM = "
  output.puts "{"

  index.times do |i|
    output.puts "  &p_macro#{i},"
  end

  output.puts "};"

end

# Create header and source files for maps
def create_individual_keymap_sourcefiles
  macroIndex = 0
  $kbp.keyboards.each do |keyboard|
    keyboard.maps.each do |map|
      modeKeys = {}
      kbIdentifier = $keymapIDs[map.id]
      header = File.new("#{$options[:outdir]}/#{kbIdentifier}.h", 'w+')
      source = File.new("#{$options[:outdir]}/#{kbIdentifier}.c", 'w+')

      if $options[:clean]
        source.close
        delete_file(source.path)
        header.close
        delete_file(header.path)
        next
      end

      source.puts "#include \"HIDUsageTables.h\""
      source.puts "#include \"ModeKeys.h\""
      source.puts
      source.puts "uint16_t kbd_map_#{kbIdentifier}[] PROGMEM = "
      source.puts "{"
      sortedKeys = map.keys.sort
      index = 0
      sortedKeys.each do |loc, key|
        locIndex = loc[1,5].to_i(16)
        while( index < locIndex )
          source.puts "  /* t#{"%x"%index} */ HID_USAGE_NONE,"
          index += 1
        end

        if !key.macro.empty?
          source.puts "  /* #{loc} */ HID_USAGE_MACRO(#{macroIndex}),"
          macroIndex += 1
          index += 1
          next
        end

        if key.mode != ''
          source.puts "  /* #{loc} */ MAKE_USAGE(HID_USAGE_PAGE_CUSTOM, MODE_KEY_#{normalize_identifier(key.mode)}),"
          modeKeys[loc] = key.mode
          $allModeKeys[locIndex] = ModeKey.new(key.mode, locIndex, keyboard.reverseMatrix[loc],
                                               key.modeType, key.modeMapID)
          index += 1
          next
        end

        next if key.usage == nil

        source.puts "  /* #{loc} */ HID_USAGE_#{normalize_identifier(key.usage.name)},"
        index += 1
      end
      source.puts "};"

      if keyboard.defaultMap == map.id
        source.puts
        source.puts "uint16_t * kbd_map_default PROGMEM = pgm_read_word(&kbd_map_#{kbIdentifier}[0]);"
      end

      # write the header file
      header.puts include_guard(header.path, :begin)
      header.puts
      header.puts "extern uint16_t kbd_map_#{kbIdentifier}[] PROGMEM;"
      header.puts

      # write inverse mappings of mode keys
      modeKeys.each do |location, key|
        header.puts "#define MODE_KEY_MATRIX_POS_#{normalize_identifier(key)} #{$invertedMatrix[location]}"
      end

      header.puts
      header.puts include_guard(header.path, :end)
    end
  end
end

def create_matrix_header
  output = File.new("#{$options[:outdir]}/matrix.h", 'w+')

  if $options[:clean]
    output.close
    delete_file(output.path)
    return
  end

  output.puts include_guard(output.path, :begin)
  output.puts
  rows = cols = 0
  $kbp.keyboards.each do |keyboard|
    matrix = keyboard.matrix
    if !matrix.empty? && !matrix[0].empty?
      rows = matrix.size
      cols = matrix[0].size
      break
    end
  end

  output.puts "#define COL(byte) (((uint8_t)byte)>>3)"
  output.puts "#define ROW(byte) (((uint8_t)byte)&~(0xff<<3))"
  output.puts "#define MATRIX_CELL(row,col) ((((uint8_t)(col))<<3)|((uint8_t)(row)))"
  output.puts 
  output.puts "#define NUM_ROWS #{rows}"
  output.puts "#define NUM_COLS #{cols}"
  output.puts
  output.puts "typedef const uint16_t * MatrixMap;"
  output.puts "typedef uint8_t Cell;"
  output.puts
  output.puts "extern const MatrixMap kbd_map_mx_default;"
  output.puts

  output.puts include_guard(output.path, :end)

end

# Create "keymaps.h" - includes all of the generated keymap headers
def create_keymaps_header
  output = File.new("#{$options[:outdir]}/keymaps.h", 'w+')

  if $options[:clean]
    output.close
    delete_file(output.path)
    return
  end

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

  if $options[:clean]
    output.close
    delete_file(output.path)
    return
  end

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
  $kbp.keyboards.each do |keyboard|
    matrix = keyboard.matrix
    if !matrix.empty? && !matrix[0].empty?
      keyboard.maps.each do |map|
        modeKeys = {}
        kbIdentifier = $keymapIDs[map.id]
        header = File.new("#{$options[:outdir]}/#{kbIdentifier}_mx.h", 'w+')
        source = File.new("#{$options[:outdir]}/#{kbIdentifier}_mx.c", 'w+')
  
        if $options[:clean]
          source.close
          delete_file(source.path)
          header.close
          delete_file(header.path)
          next
        end
  
        rows = matrix.size
        cols = matrix[0].size

        header.puts include_guard(header.path, :begin)
        header.puts
        header.puts '#include "config.h"'
        header.puts '#include "matrix.h"'
        if map.type == 'system'
          storage = "PROGMEM"
          header.puts '#include <avr/pgmspace.h>'
        else
          storage = "EEMEM"
          header.puts '#include <avr/eeprom.h>'
        end
        header.puts
        header.puts "#define NUM_ROWS #{rows}"
        header.puts "#define NUM_COLS #{cols}"
        header.puts
        header.puts "extern const uint16_t kbd_map_#{kbIdentifier}_mx[] #{storage};"
        header.puts "extern const MatrixMap kbd_map_mx_default PROGMEM;"
        header.puts
        header.puts include_guard(header.path, :end)


        source.puts "#include \"#{header.path}\""

        source.puts '#include "HIDUsageTables.h"'
        source.puts '#include "ModeKeys.h"'
        source.puts
        source.print "const uint16_t kbd_map_#{kbIdentifier}_mx[] #{storage} ="

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

        source.puts
        source.puts "{"
        flippedMatrix.each_index do |iCol|
          if iCol > 0
            source.puts ","
            source.puts
          end
          source.puts "\t/* col: #{iCol} */"
          source.puts
          row = flippedMatrix[iCol]
          row.each_index do |iRow|
            location = row[iRow]
            source.puts "," if iRow != 0
            source.print "\t"
            source.print "/* row:#{iRow} loc = #{location} */ "
            key = map.keys[location]
            if key == nil
              source.print "HID_USAGE_NONE"
            elsif !key.macro.empty?
              source.print "HID_USAGE_MACRO(#{macroIndex})"
              macroIndex += 1
            elsif key.mode != ''
              source.print "MAKE_USAGE(HID_USAGE_PAGE_CUSTOM, MODE_KEY_#{normalize_identifier(key.mode)})"
            elsif key.usage == nil
              source.print "HID_USAGE_NONE"
            else
              source.print "HID_USAGE_#{normalize_identifier(key.usage.name)}"
            end
          end
        end

        source.puts
        source.puts "};"

        if keyboard.defaultMap == map.id
          source.puts
          source.puts "const MatrixMap kbd_map_mx_default PROGMEM = kbd_map_#{kbIdentifier}_mx;"
        end

      end
    end
  end
end

# Create "ModeKeys.h" - defines the mode key tables, one set for each map
def create_modekeys_header
  if $options[:clean]
    delete_file("#{$options[:outdir]}/ModeKeys.h")
    delete_file("#{$options[:outdir]}/ModeKeys.c")
  end


  macroIndex = 0
  $kbp.keyboards.each do |keyboard|
    keyboard.maps.each do |map|
      modeKeys = {}
      kbIdentifier = $keymapIDs[map.id]

      sortedKeys = map.keys.sort
      index = 0
      sortedKeys.each do |loc, key|
        locIndex = loc[1,5].to_i(16)
        while( index < locIndex )
          index += 1
        end

        if !key.macro.empty?
          macroIndex += 1
          index += 1
          next
        end

        if key.mode != ''
          modeKeys[loc] = key.mode
          $allModeKeys[locIndex] = ModeKey.new(key.mode, locIndex, keyboard.reverseMatrix[loc],
                                               key.modeType, key.modeMapID)
          index += 1
          next
        end

        next if key.usage == nil

        index += 1
      end
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

$options = { :clean => false }
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
  opts.on("--clean") do |clean|
    $options[:clean] = true
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
  create_matrix_header
  create_hid_usage_tables_header
  #create_individual_keymap_sourcefiles
  create_individual_matrix_map_sourcefiles
  create_keymaps_header
  create_modekeys_header
  create_macro_header
rescue Exception => e
  puts e.to_str
  raise
end
