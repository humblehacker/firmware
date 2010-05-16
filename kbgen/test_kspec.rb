#!/usr/bin/env ruby

require 'rubygems'
require 'treetop'
require 'kspec'

require 'KbInternals.rb'
require 'Keyboard.rb'
require 'Utils.rb'

filename = '../tm_prototype_2/layouts/kb.kspec'
inputfile = File.open(filename).read
parser = KSpecParser.new
if (parser.parse(inputfile))
  puts "#{filename} success!"
else
  puts "#{filename} failure"
  puts " line  : #{parser.failure_line}"
  puts " column: #{parser.failure_column}"
  puts " reason: #{parser.failure_reason}•"
end


