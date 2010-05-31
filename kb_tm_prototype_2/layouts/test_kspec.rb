#!/usr/bin/env ruby
require 'rubygems'
require 'treetop'
require 'kspec'
require 'hid'

class MapNode < Treetop::Runtime::SyntaxNode
end

class HIDUsageTable
  attr_accessor :pages, :usages
  def initialize
    @pages = []
    @usages = []
  end
end

class UsagePage
  attr_reader :name, :id
  def initialize(name, id)
    @name = name
    @id = id
  end
end

class Usage
  attr_reader :name, :id, :page
  def initialize(name, id, page)
    @name = name
    @id = id
    @page = page
  end
end

parser = KSpecParser.new
hidparser = HIDParser.new
require 'PP'

hidfile = File.open('HIDUsageTables.kspec')
result = hidparser.parse(hidfile.read)

table = HIDUsageTable.new
result.build_table(table)

pp table

exit

inputfile = File.open('Map-Common.kspec').read
puts parser.parse(inputfile)

# single mapping
test = <<-EOF
Map:Common rev:xxx 
  Key:1A Mapping:ESCAPE tl:"esc"
EOF
puts parser.parse(test)

# multiple keys with single mappings
test = <<-EOF
Map:Common rev:xxx 
  Key:1A Mapping:ESCAPE tl:"esc"
  Key:1B Mapping:F1     tl:"F1"
EOF
puts parser.parse(test)

# single key with multiple mappings
test = <<-EOF
Map:Common rev:xxx 
  Key:1A Mapping:ESCAPE    tl:"esc"
         <shift>Mapping:F1 tl:"F1"
EOF
puts parser.parse(test)

# multiple keys with multiple mappings
test = <<-EOF
Map:Common rev:xxx 
  Key:1A Mapping:ESCAPE    tl:"esc"
  Key:1B Mapping:F1        tl:"F1"
  Key:1C Mapping:ESCAPE    tl:"esc"
         <shift>Mapping:F1 tl:"F1"
  Key:1D Mapping:F1        tl:"F1"
EOF
puts parser.parse(test)

# Mode keys
test = <<-EOF
Map:Common rev:xxx 
  Key:1A Mode:Dv type:toggle tl:"DV"
         <shift>Mode:Nm led:Num type:toggle tl:"Num"
EOF
puts parser.parse(test)
