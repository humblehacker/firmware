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

def generate_from_template(filename, template, bindings)
  block = <<-EOF
    input  = File.new("../kbgen/templates/#{template}")
    output = File.new("#{$options[:outdir]}/#{filename}", 'w+')
    puts "Generating: \#{output.path} from \#{input.path}" if $options[:debug]
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
