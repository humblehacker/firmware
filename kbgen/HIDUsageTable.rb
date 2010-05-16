#!/usr/bin/env ruby
#
#  Created by David Whetstone on 2007-03-25.
#  Copyright (c) 2007. All rights reserved.

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
