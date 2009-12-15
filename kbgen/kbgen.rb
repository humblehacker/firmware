#!/usr/bin/env ruby
#
#  Created by David Whetstone on 2007-03-24.
#  Copyright (c) 2007. All rights reserved.

require 'rubygems'
require 'HIDUsageTable.rb'
require 'Keyboard.rb'
require 'KbParser.rb'
require 'KbRender.rb'
require 'optparse'

includes = Includes.new

options = { :showInternalIDs => true }
opts = OptionParser.new do |opts|
  opts.on("-I [INCLUDE]") do |path|
    includes.push path
  end
  opts.on("-x", "--no-internal-ids", "Do not show internal ids") do |noInt|
    options[:showInternalIDs] = false
  end
  opts.on("-d", "--debug", "Turn on debugging info") do |debug|
    options[:debug] = true
    puts "Debugging on"
  end
end

begin
  opts.parse!(ARGV)
rescue Exception => e
  puts e, "", opts
  exit 1
end

filename = opts.default_argv[0]

# Parse the HID Usage Tables
hid = HIDUsageTable.new('HIDUsageTables.xml', includes)

# process
kbp = KbParser.new(hid, includes, options)
kbp.process_keymap filename

kbp.keyboards.each do |keyboard|
  renderer = KBRender.new(keyboard, options)
  renderer.render(:usage_changes)
  renderer.export
end

# require 'wx'
# class KeyboardApp < Wx::App
#   def on_init
#     @mainFrame = Wx::Frame.new(nil, -1, "Title")
#     @button = Wx::BitmapButton.new(@mainFrame, -1, Wx::Bitmap.new(10, 10))
#     @button.show
#     @mainFrame.show
#   end
# end
# 
# KeyboardApp.new.main_loop



