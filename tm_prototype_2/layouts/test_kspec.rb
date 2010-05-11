#!/usr/bin/env ruby
require 'rubygems'
require 'treetop'
require 'kspec'
require 'hid'

class MapNode < Treetop::Runtime::SyntaxNode
end

class HIDUsageTableNode < Treetop::Runtime::SyntaxNode
end

parser = KSpecParser.new
hidparser = HIDParser.new

hidfile = File.open('HIDUsageTables.kspec')
puts hidparser.parse(hidfile.read)

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
