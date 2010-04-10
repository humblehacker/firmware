#!/usr/bin/env ruby
#
#  Created by David Whetstone on 2007-03-25.
#  Copyright (c) 2007. All rights reserved.

require 'rubygems'
require 'xml/libxml'
require 'optparse'
require 'PP'
require 'Utils.rb'

class UsagePage
  attr_accessor :name, :id, :usages
  def initialize
    @name = ''
    @id = 0
    @usages = []
  end
end

class Usage
  attr_accessor :page, :id, :name
  def initialize(usagePage)
    @page = usagePage
    @id   = 0
    @name = ''
  end
end

class HIDUsageTable
  attr_reader :usagePages, :usagePagesByName, :usagesByName
  def initialize(filename, includes)
    @includes = includes

    if !File.exist? filename
      newname = @includes.find_file(filename)
      if newname == nil
        raise "ERROR: Could not find file #{filename}"
      end
      filename = newname
    end
    hidParser = XML::SaxParser.file(filename)
    hidParser.callbacks = HidHandler.new
    hidParser.parse
    @usagePagesByName = hidParser.callbacks.usagePagesByName
    @usagesByName     = hidParser.callbacks.usagesByName
    @usagePages       = hidParser.callbacks.usagePages
  end
end

class HidHandler
  
  include XML::SaxParser::Callbacks
  attr_reader :usagePagesByName, :usagesByName, :usagePages
  
  
  def initialize
    @currentUsage = nil
    @currentUsagePage = nil
    @content = nil
    @state = [:Root]
    
    @usagePagesByName = {}
    @usagesByName     = {}
    @usagePages       = []
  end
     
  def on_start_element(name, attr_hash)
    case @state.last  
    when :Root
      case name
      when "HID"
        @state.push :HID
      end
    when :HID
      case name
      when "UsagePages"
        @state.push :UsagePages
      end
    when :UsagePages
      case name
      when "UsagePage"
        @state.push :UsagePage
        @currentUsagePage = UsagePage.new
      end
    when :UsagePage
      case name
      when "Name"
        @content = lambda { |chars| @currentUsagePage.name += chars }
      when "ID"
        @content = lambda { |chars| @currentUsagePage.id = normalize_number(chars) }
      when "UsageID"
        @currentUsage = Usage.new(@currentUsagePage)
        @state.push :UsageID
      end
    when :UsageID
      case name
      when "Name"
        @content = lambda { |chars| @currentUsage.name += chars }
      when "ID"
        @content = lambda { |chars| @currentUsage.id = normalize_number(chars) }
      end
    end
  end

  def on_characters(chars)
    if @content != nil
      @content.call(chars)
    end
  end

  def on_end_element(name)
    case @state.last
    when :Root
    when :HID
      case name
      when "HID"
        @state.pop
      end
    when :UsagePages
      case name
      when "UsagePages"
        @state.pop
        @currentUsagePage = nil
      end
    when :UsagePage
      case name
      when "UsagePage"
        if @usagePagesByName.has_key? @currentUsagePage.name == true
          raise "Error: Duplicate name: #{@currentUsagePage.name} for #{@currentUsagePage.id} and #{@usagePagesByName[@currentUsagePage.name]}"
        end 
        @usagePagesByName[@currentUsagePage.name] = @currentUsagePage
        @usagePages.push @currentUsagePage
        @currentUsagePage = nil
        @state.pop
      when "Name", "ID"
        @content = nil
      end
    when :UsageID
      case name
      when "UsageID"
        add_usage(@currentUsage, @currentUsagePage)
        @content = nil
        @state.pop
      when "Name", "ID"
        @content = nil
      end
    end
  end
  
  def on_end_document
    @usagePages.sort! { |a,b| a.id <=> b.id }
    @usagePages.each do |usagePage|
      usagePage.usages.sort! { |a,b| a.id <=> b.id }
    end
  end

  def add_usage(usage, page)
    if @usagesByName.has_key? usage.name == true
      raise "Error: Duplicate name: #{usage.name} for #{usage.id} and #{@usagePagesByName[page.name]}"
    end
    @usagesByName[usage.name] = usage
    page.usages.push usage
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
