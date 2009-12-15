#!/usr/bin/env ruby
#
#  Created by David Whetstone on 2007-03-28.
#  Copyright (c) 2007. All rights reserved.

require 'rubygems'
require 'appscript'
require 'facet/String/word_wrap'
include Appscript

class KBRender
  def initialize(kb, options)
    @kb = kb
    @options = options
    @@Offset = Point.new( 1 * Keyboard::Scale, 3 * Keyboard::Scale )
    @show_overrides = :none
    @specialRegex = /\{[a-z]+\}/

    # Create a new OmniGraffle Document
    @og = app.by_id('com.omnigroup.OmniGrafflePro')
    @og.make( :new => :document,
              :with_properties => { :template => 'US Legal (Metric)'} )
    @ogDoc = @og.windows.first.get
    @og.windows.first.zoomed.set true
    
    # Create the layers
    @layers = {}
    @kb.maps.each do |map|
      make_layer(map.id)
    end
    if @options[:showInternalIDs]
      make_layer('InternalIDs', true)
    end
    make_layer('Background', true)
    make_layer('Keyboard', true)
    make_layer('Mounts', true)
    
    @fontAdj = 0
    @fontAdj = 2 if !@options[:showInternalIDs]
    
    # Define special characters
    @special = {
      '{up}'        => { :text => '↑', :size => 14 + @fontAdj },
      '{down}'      => { :text => '↓', :size => 14 + @fontAdj },
      '{left}'      => { :text => '←', :size => 14 + @fontAdj },
      '{right}'     => { :text => '→', :size => 14 + @fontAdj },
      '{web}'       => { :text => '', :size => 18 + @fontAdj, :font => "Webdings" },
      '{mail}'      => { :text => '', :size => 18 + @fontAdj, :font => "Webdings" },
      '{phone}'     => { :text => '☎', :size => 14 + @fontAdj },
      '{calc}'      => { :text => '', :size => 18 + @fontAdj, :font => "Webdings" },
      '{del}'       => { :text => '⌦', :size => 12 + @fontAdj },
      '{command}'   => { :text => '⌘', :size => 12 + @fontAdj },
      '{win}'       => { :text => '', :size => 18 + @fontAdj, :font => 'Wingdings'},
      '{copyright}' => { :text => '©', :size => 12 + @fontAdj },
      '{minus}'     => { :text => '–', :size => 12 + @fontAdj },
      '{bump}'      => { :text => '○', :size =>  8 + @fontAdj },
      '{tunes}'     => { :text => '♫', :size => 14 + @fontAdj }
    }

    replace_special(@kb.copyright)
  end
  
  def make_layer(name, visible=false)
    lprops = { :name    => name,
               :locked  => false, 
               :prints  => true,  
               :visible => visible }
    layer = @og.make(:new => :layer,
                     :at => @ogDoc.canvases.end,
                     :with_properties => lprops)
    @layers[name] = layer
  end
  
  
  def render(show_overrides=:none)
    @show_overrides = show_overrides
    draw_headers
    draw_mounts
    draw_keyboard
    draw_labels
    @layers
    @show_overrides = :none
  end
  
  def export(show_overrides=:none)
    @show_overrides = show_overrides
    # do the export
    @show_overrides = :none
  end
  
  def draw_text(text, size, alignment, origin, layer)
    black = rgb_to_og(@kb.colors['Black'])
    properties = 
    {
      :vertical_padding  => 0,
      :side_padding      => 0,
      :draws_stroke      => false,
      :draws_shadow      => false,
      :autosizing        => :full,
      :fill              => :no_fill,
      :origin            => origin,
      :text_placement    => :center,

      :text =>
      { 
        :text  => text,
        :size  => size + @fontAdj,
        :font  => 'ArialRoundedMTBold',
        :color => black,
        :alignment => alignment
      }
    }

    label = @og.make( :new             => :shape, 
                      :at              => @ogDoc.graphics.end,
                      :with_properties => properties )
    label.layer.set layer
    label.origin.set origin
    return label
  end
  

  def draw_headers
    textBox = draw_text("Layout '#{@kb.layout.id}': Revision #{@kb.layout.rev}",
                        10, :right, [10000,0], @layers['Keyboard'])
    baseline_origin =  [@kb.max.x - textBox.size.x.get + @@Offset.x, 
                        @kb.max.y + @@Offset.y + 0.5 * Keyboard::Scale]
    textBox.origin.set baseline_origin

    @kb.maps.each do |map|
      layer = @layers[map.id]
      textBox = draw_text("#{@kb.org} #{@kb.layout.id}-#{map.id}  #{@kb.platform}", 14,
                          :left, [1 * Keyboard::Scale, 1 * Keyboard::Scale], 
                          layer)
      foot = ''
      map.ids.each_index do |n|
        foot += "Map '#{map.ids[n]}': Revision #{map.revs[n]}\n" 
      end
      textBox = draw_text(foot, 10, :right, [10000,0], layer)
      textBox.origin.set [@kb.max.x - textBox.size.x.get + @@Offset.x, 
                          @kb.max.y + @@Offset.y + 0.9 * Keyboard::Scale]
    end
    textBox = draw_text( @kb.copyright, 10, :left, [10000,0], @layers['Keyboard'])
    textBox.origin.set [1 * Keyboard::Scale, @kb.max.y + @@Offset.y + 0.5 * Keyboard::Scale]
    
    baseline_origin[0] = 30.25
    baseline_origin[1] += 5
    shape = @og.make( :new => :shape,
                      :at  => @ogDoc.graphics.end,
                      :with_properties => { :double_stroke => true, :thickness => 4, :draws_shadow => false,
                                            :origin => baseline_origin, :size => [200, 64], :fill => :no_fill } )
    shape.layer.set @layers['Keyboard']
    
    shape = @og.make( :new => :shape,
                      :at  => @ogDoc.graphics.end,
                      :with_properties => 
                      { 
                        :text => 
                        [
                          {:size => 10, :font => "DejaVuSans",      :text => "\n"}, 
                          {:size => 9,  :font => "DejaVuSans-Bold", :text => "LA"}, 
                          {:size => 9,  :font => "DejaVuSans",      :text => ": Left Alt\n"}, 
                          {:size => 9,  :font => "DejaVuSans-Bold", :text => "LC"}, 
                          {:size => 9,  :font => "DejaVuSans",      :text => ": Left Control\n"}, 
                          {:size => 9,  :font => "DejaVuSans-Bold", :text => "LS"}, 
                          {:size => 9,  :font => "DejaVuSans",      :text => ": Left Shift\n"}, 
                          {:size => 9,  :font => "DejaVuSans-Bold", :text => "LG"}, 
                          {:size => 9,  :font => "DejaVuSans",      :text => ": Left GUI (Win)"}
                        ],
                        :vertical_padding => 5, :autosizing => :vertically_only, 
                        :draws_stroke => false, :double_stroke => true, 
                        :thickness => 4, :draws_shadow => false, 
                        :origin => [131, 465], :size => [101, 61], :fill => :no_fill
                      }
                    )
                        
    shape = @og.make( :new => :shape,
                      :at  => @ogDoc.graphics.end,
                      :with_properties => 
                      { 
                        :text => 
                        [
                          {:size => 10, :font => "DejaVuSans-Bold", :text => "Legend\n"}, 
                          {:size => 9,  :font => "DejaVuSans-Bold", :text => "RA"}, 
                          {:size => 9,  :font => "DejaVuSans",      :text => ": Right Alt\n"}, 
                          {:size => 9,  :font => "DejaVuSans-Bold", :text => "RC"}, 
                          {:size => 9,  :font => "DejaVuSans",      :text => ": Right Control\n"}, 
                          {:size => 9,  :font => "DejaVuSans-Bold", :text => "RS"}, 
                          {:size => 9,  :font => "DejaVuSans",      :text => ": Right Shift\n"}, 
                          {:size => 9,  :font => "DejaVuSans-Bold", :text => "RG"}, 
                          {:size => 9,  :font => "DejaVuSans",      :text => ": Right GUI (Win)"}
                        ],
                        :vertical_padding => 5, :autosizing => :vertically_only, 
                        :draws_stroke => false, :double_stroke => true, 
                        :thickness => 4, :draws_shadow => false, 
                        :origin => [34, 465], :size => [101, 61], :fill => :no_fill
                      }
                    )
    #     make new shape at end of graphics with properties {:text {{:size 10, :font "DejaVuSans-Bold", :text "Legend"}, {:size 10, :font "ArialRoundedMTBold", :text ":"}, {:size 9, :font "ArialRoundedMTBold", :text "
    # "}, {:size 9, :font "DejaVuSans-Bold", :text "RA"}, {:size 9, :font "DejaVuSans", :text ": Right Alt
    # "}, {:size 9, :font "DejaVuSans-Bold", :text "RC"}, {:size 9, :font "DejaVuSans", :text ": Right Control
    # "}, {:size 9, :font "DejaVuSans-Bold", :text "RS"}, {:size 9, :font "DejaVuSans", :text ": Right Shift
    # "}, {:size 9, :font "DejaVuSans-Bold", :text "RG"}, {:size 9, :font "DejaVuSans", :text ": Right GUI (Win)"}}, vertical padding: 5, autosizing: vertically only, draws stroke: false, double stroke: true, thickness: 4, draws shadow: false, origin: {33.771835, 465.464386}, :size {103.685165, 61.000000}, fill: no fill}
    #     make new shape at end of graphics with properties {vertical padding: 5, double stroke: true, thickness: 4, 
    #       draws shadow: false, origin: {30.252129, 464.747864}, :size {201.944901, 63.999950}, fill: no fill}
    #     assemble graphics -3 through -1
    #   end tell
    # end tell
    
  end
  
  def draw_keyboard
    showInt = @options[:showInternalIDs]
    black = rgb_to_og(@kb.colors['Black'])
    white = rgb_to_og(@kb.colors['White'])
    brown = rgb_to_og(@kb.colors['Brown'])
    keyboardLayer = @layers['Keyboard']
    idLayer = @layers['InternalIDs'] if showInt
    bgLayer = @layers['Background']
    @kb.layout.rows.each do |row|
      if row != nil
        row.keydefs.to_a.each do |keydef|
          if keydef.null == false
            # maybe draw bump
            draw_label(keydef, '{bump}', bgLayer, :center, :center, black) if keydef.bump

            # draw background
            color = keydef.background != nil ? rgb_to_og(keydef.background) : white
            draw_background(keydef, color)
            
            # draw border
            draw_border(keydef, keyboardLayer, black, true)
            
            # maybe draw Internal ID
            draw_label(keydef, keydef.id, idLayer, :left, :bottom, brown) if showInt
          end
        end
      end
    end
  end

  def draw_labels
    black = rgb_to_og(@kb.colors['Black'])
    turq  = rgb_to_og(@kb.colors['Turquoise'])
    red   = rgb_to_og(@kb.colors['Red'])
    layer = nil
    platformMap = nil
    @kb.maps.each do |map|
      if layer != nil
        layer.visible.set false
      end
      layer = @layers[map.id]
      layer.visible.set true
      map.keys.each do |location, key|

        next if key.empty?
        keydef = @kb.key_def_from_ID(location)
        color = key.color != nil ? rgb_to_og(key.color) : black
        # maybe draw colored border
        if @show_overrides != :none
          if (@show_overrides == :all && key.overridden?) || (@show_overrides == :usage_changes && key.usageChanged?)
            draw_border(keydef, layer, red)
          end
        end

        # draw regular labels
        key.label.legends.each do |loc, legend|
          case loc 
          when :topLeft, :topCenter, :topRight
            placement = :top                  
          when :centerLeft, :center, :centerRight
            # :center... options are the same as :bottom... options
            # when :showInternalIDs is false
            placement = :center
          when :bottomLeft, :bottomCenter, :bottomRight
            if @options[:showInternalIDs]
              placement = :center
            else
              placement = :bottom
            end
          end
          case loc
          when :topLeft, :centerLeft, :bottomLeft
            alignment = :left
          when :topCenter, :center, :bottomCenter
            alignment = :center
          when :topRight, :centerRight, :bottomRight
            alignment = :right
          end
          legendColor = legend.color == nil ? color : rgb_to_og(legend.color)
          draw_label(keydef, legend.text, layer, alignment, placement, legendColor, legend.note)
        end
        
        # draw Usage ID
        usageID = ''
        if key.usage != nil
          usageID = ('%x'%key.usage.id).to_s.rjust(2, "0")
          color = turq if key.usage != nil && key.usage.page.id == 0x0c
          draw_label(keydef, usageID, layer, :right, :bottom, color)
        elsif !key.macro.empty?
          text = ''
          usageID = ''
          key.macro.each do |macro|
            if key.macro.last.modifiers != 0
              usageID += "#{ModifierCodes[:L_ALT]} "  if (key.macro.last.modifiers & Modifiers[:L_ALT] ) != 0
              usageID += "#{ModifierCodes[:L_CTRL]} " if (key.macro.last.modifiers & Modifiers[:L_CTRL]) != 0
              usageID += "#{ModifierCodes[:L_SHFT]} " if (key.macro.last.modifiers & Modifiers[:L_SHFT]) != 0
              usageID += "#{ModifierCodes[:L_GUI]} "  if (key.macro.last.modifiers & Modifiers[:L_GUI]) != 0
              usageID += "#{ModifierCodes[:R_ALT]} "  if (key.macro.last.modifiers & Modifiers[:R_ALT] ) != 0
              usageID += "#{ModifierCodes[:R_CTRL]} " if (key.macro.last.modifiers & Modifiers[:R_CTRL]) != 0
              usageID += "#{ModifierCodes[:R_SHFT]} " if (key.macro.last.modifiers & Modifiers[:R_SHFT]) != 0
              usageID += "#{ModifierCodes[:R_GUI]} "  if (key.macro.last.modifiers & Modifiers[:R_GUI]) != 0
            end
            usageID += ('%x'%key.macro.last.usage.id).to_s.rjust(2, "0")
            usageID += " "
          end
          usageID.chomp!
          usageID.reverse!
          usageID.word_wrap!(6)
          usageID.reverse!
          color = key.color != nil ? rgb_to_og(key.color) : black
          # color = turq if key.usage != nil && key.usage.page.id == 0x0c
          draw_label(keydef, usageID, layer, :right, :bottom, color)
        end
      end
    end
  end
  
  def draw_border(keydef, layer, color, shadow=false)
    properties = 
    {
      :origin        => (keydef.origin + @@Offset).to_a, 
      :size          => keydef.size.to_a,
      :fill          => :no_fill,
      :draws_shadow  => shadow,
      :corner_radius => 2,
      :stroke_color  => color
    }
    shape = @og.make( :new             => :shape, 
                      :at              => @ogDoc.graphics.end,
                      :with_properties => properties )
    shape.layer.set layer
    
  end
  
  def draw_mount(mount, keydef, layer)
    black = rgb_to_og(@kb.colors['Black'])
    spacing = @kb.layout.spacing
    mountdef = keydef.dup
    mountdef.origin += @@Offset
    mountdef.origin.y += (keydef.size.height - mount.height) / 2
    numMounts = ((keydef.size.width - (2 * spacing)) / mount.width).to_i
    subWidth = (keydef.size.width - ((numMounts-1) * spacing)) / numMounts
    padding = (subWidth - mount.width) / 2
    numMounts.times do |time|
      mountdef.origin.x += (time*(mount.width+padding+spacing)) + padding
      properties = 
      {
        :origin => mountdef.origin.to_a,
        :size => mount.to_a,
        :fill => :no_fill,
        :draws_shadow => false,
        :stroke_color => black
      }
      shape = @og.make( :new             => :shape,
                        :at              => @ogDoc.graphics.end,
                        :with_properties => properties )
      shape.layer.set layer
    end
  end
  
  def replace_special(text)
    if text =~ @specialRegex 
      replaceMe = text.match(@specialRegex)[0]
      if @special.has_key? replaceMe
        special = @special[replaceMe]
        text.sub!(@specialRegex, special[:text])
        return special
      end
    end
    return nil
  end
   
  def draw_label(keydef, label, layer, alignment, placement, color, note=nil)
    properties = 
    {
      :vertical_padding  => 5,
      :draws_stroke      => false,
      :draws_shadow      => false,
      :fill              => :no_fill,
      :size              => keydef.size.to_a,
      :origin            => (keydef.origin + @@Offset).to_a,
      :text_placement    => placement,
      :interline_spacing => -5.0,
      :side_padding      => (alignment==:center)?3:5,
      
      :text =>
      { 
        :text  => label,
        :size  => 9 + @fontAdj,
        :font  => 'ArialRoundedMTBold',
        :color => color,
        :alignment => alignment
      }
    }
    
    if note != nil
      properties[:notes] = {:text => note }
    end
    
    special = replace_special(label)
    if special != nil
      properties[:text][:text] = label
      properties[:text][:font] = special[:font] if special.has_key? :font
      properties[:text][:size] = special[:size] if special.has_key? :size
    end

    label = @og.make( :new             => :shape, 
                      :at              => @ogDoc.graphics.end,
                      :with_properties => properties )
    # WORKAROUND: setting interline_spacing using make :with_properties
    # doesn't seem to work
    if @options[:showInternalIDs]
      label.interline_spacing.set -5.0 
    end
    label.layer.set layer
  end
  
  def draw_mounts
    return if @kb.layout.mount == nil

    layer = @layers['Mounts']
    mount = @kb.layout.mount
    @kb.layout.rows.each do |row|
      if row != nil
        row.keydefs.to_a.each do |keydef|
          if keydef.null == false
            
            # draw border
            draw_mount(mount, keydef, layer)

          end
        end
      end
    end
    
  end
  
  def rgb_to_og(color)
    r = color[0..1].to_i(16).to_f
    g = color[2..3].to_i(16).to_f
    b = color[4..5].to_i(16).to_f
    return [r/0xff,g/0xff,b/0xff]
  end
  
  def draw_background(keydef, color)
    properties = 
    {
      :draws_stroke      => true,
      :draws_shadow      => false,
      # :fill              => :radial_fill,
      :fill_color        => color,
      # :gradient_color    => ???,
      :size              => keydef.size.to_a,
      :origin            => (keydef.origin + @@Offset).to_a,
      :corner_radius     => 2,
      :layer             => @bgLayer
    }

    shape = @og.make( :new             => :shape, 
                      :at              => @ogDoc.graphics.end,
                      :with_properties => properties )
    shape.layer.set @layers['Background']
  end
  
end
