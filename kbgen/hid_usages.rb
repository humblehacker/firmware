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

require 'rubygems'
require 'optparse'
require 'Utils.rb'
require 'treetop'
require 'kspec'
require 'hid'

class UsagePage
  attr_accessor :name, :id
  def initialize(name, id)
    @name = name
    @id = id
  end
end

class Usage
  attr_accessor :name, :id, :page
  def initialize(name, id, page)
    @page = page
    @id   = id
    @name = name
  end
end

class HIDUsageTable
  attr_accessor :pages, :usages, :usagePagesByName, :usagesByName, :usagesByPage
  def initialize(filename, includes)
    @pages = []
    @usages = []
    @includes = includes

    if !File.exist? filename
      newname = @includes.find_file(filename)
      if newname == nil
        raise "ERROR: Could not find file #{filename}"
      end
      filename = newname
    end
    hidfile = File.open(filename)
    hid_parser = HIDParser.new
    result = hid_parser.parse(hidfile.read)
    result.build_table(self)

    @usagePagesByName = {}
    @usagesByPage = {}
    @usagesByName = {}

    @pages.each do |page|
      @usagePagesByName[page.name] = page
      @usagesByPage[page] = Array.new
    end
    @usages.each {|usage| @usagesByName[usage.name] = usage}
    @usages.each {|usage| @usagesByPage[usage.page] << usage}
  end
end

if __FILE__ == $0
  includes = Includes.new

  opts = OptionParser.new do |opts|
    opts.on("-I [INCLUDE]") do |path|
      puts path
      includes.push path
    end
  end

  begin
    opts.parse!(ARGV)
  rescue Exception => e
    puts e, "", opts
    exit 1
  end

  filename = opts.default_argv[0]
  if filename == nil
    puts "Please specify a filename"
    exit 1
  end

  # Parse the HID Usage Tables
  hid = HIDUsageTable.new(filename, includes)
  pp hid
end
