require 'xml/libxml'
require 'shell'
require 'kbInternals.rb'
require 'Keyboard.rb'
require 'Utils.rb'

class KbParser
  attr_reader :keyboards, :max
  def initialize(hid, includes, options, filename)
    if !File.exist?(filename)
      newname = includes.find_file(filename)
      if newname == nil
        raise "ERROR: Could not find include file #{filename}"
      end
      filename = newname
    end
    puts "Parsing #{filename}" if options[:debug]
    kbdParser = XML::SaxParser.file(filename)
    handler = KeyboardCH.new(filename, options, hid, includes)
    kbdParser.callbacks = handler
    kbdParser.parse
    @keyboards = handler.keyboards
  end

end

class KeyboardCH
  include XML::SaxParser::Callbacks

  attr_reader :keyboards, :filenames
  
  Scale = 28.34627813368

  def initialize(filename, options, hid, includes)
    @level = 0
    @options = options
    @includes = includes
    @state = [:Root]
    @keyboards = []
    @hid = hid
    @filenames = [filename]
    
    @content = nil
    @currentKeyDef = nil
    @currentRowDef = nil
    @currentRow = 0
    @pos = Point.new( 0.0, 0.0 )
    @currentKeyMap = nil
    @currentKey = nil
    @currentColor = nil
    @defaultColor = nil
    @currentUsagePage = ''
    @currentUsageID = ''
    @currentMatrixRow = 0
  end

  def include(filename)
    if !File.exist?(filename)
      newname = @includes.find_file(filename)
      if newname == nil
        raise "ERROR: Could not find include file #{filename}"
      end
      filename = newname
    end
    puts "#{' '*@level}Parsing #{filename}" if @options[:debug]
    parser = XML::SaxParser.file(filename)
    parser.callbacks = self
    @filenames.push(filename)
    @level += 1
    parser.parse
    @filenames.pop
    @level -= 1
  end

  def extract_rev(rev)
    /\$Rev: ([0-9]+) \$/.match(rev)[1]
  end

  def process_modifier(attr_hash)
    case attr_hash['id']
    when "Left Alt"
      return Modifiers[:L_ALT]
    when "Left Ctrl"
      return Modifiers[:L_CTRL]
    when "Left Shift"
      return Modifiers[:L_SHFT]
    when "Left GUI"
      return Modifiers[:L_GUI]
    when "Right Alt"
      return Modifiers[:R_ALT]
    when "Right Ctrl"
      return Modifiers[:R_CTRL]
    when "Right Shift"
      return Modifiers[:R_SHFT]
    when "Right GUI"
      return Modifiers[:R_GUI]
    else
      raise UnknownModifierError.new(attr_hash['id'], @filenames)
    end
  end

  def process_usage(location, attr_hash)
    # Get the Usage Page
    # since usage names are unique across all pages, the usage page
    # specified is only used as a sanity check against the page
    # retrieved from the usage tables.
    usagePage = 'Keyboard and Keypad'
    usagePage = attr_hash['page'] if attr_hash.has_key? 'page'

    if ! @hid.usagePagesByName.has_key? usagePage
      raise "ERROR: Key #{location} while looking up '#{usagePage}'"
    end
    usagePage = @hid.usagePagesByName[usagePage]

    # Get the Usage ID
    usageID = attr_hash['id']
    usageID.gsub!("&#38;", "&")  # FIXME: where is conversion from & to $#38; happening?
    if ! @hid.usagesByName.has_key? usageID
      raise "ERROR: Key #{location} while looking up '#{usageID}'"
    end
    usage = @hid.usagesByName[usageID]
    if usage.page != usagePage # sanity check
      raise "ERROR: Usage page mismatch for #{usageID}\nShould be #{usage.page}, but #{usagePage} was specified."
    end
    return usage
  end

  def on_start_element(name, attr_hash)

    # next if name == nil
    @level += 1
    puts "#{' '*@level}<#{name} #{attr_hash}>" if @options[:debug]

    case @state.last

    # Root elements
    when :Root, :Keyboards
      case name
      when "Keyboards"
        @state.push :Keyboards
      when "Keyboard"
        @keyboards.push Keyboard.new(attr_hash['org'])
        @currentKb = @keyboards.last
        @currentKb.copyright = @copyright
        @state.push :Keyboard
      when "xi:include", "include"
        filename = attr_hash['href']
        self.include(filename)
      when "Copyright"
        @state.push :Copyright
        @content = lambda { |chars| @copyright += chars }
      else
        raise UnknownElementError.new(name, @state, attr_hash, @filenames)
      end

    # Root element or <Keyboards> sub-element
    when :Keyboard
      case name
      when "Layout"
        id  = attr_hash['id']
        rev = extract_rev(attr_hash['rev'])
        @currentKb.layout = KeyLayout.new(id, rev)
        @state.push :Layout
      when "Map"
        id  = attr_hash['id']
        rev = extract_rev(attr_hash['rev'])
        map_type = attr_hash.has_key?('type') ? attr_hash['type'] : 'system'
        defaultMap = attr_hash['default-map']
        @currentKb.defaultMap = id if defaultMap == "true"
        if !attr_hash.has_key? 'base'
          @currentKb.maps.push KeyMap.new(id, rev, map_type)
        else
          filename = attr_hash['base']
          self.include(filename)
          @currentKb.maps.last.add_map(id, rev, map_type)
        end
        @currentKeyMap = @currentKb.maps.last
        @state.push :Map
      when "Matrix"
        @currentKb.matrixId=attr_hash['id']
        @state.push :Matrix
      when "xi:include", "include"
        filename = attr_hash['href']
        self.include(filename)
      else
        raise UnknownElementError.new(name, @state, attr_hash, @filenames)
      end

    # <Keyboard> sub-elements
    when :Layout
      case name
      when "Spacing"
        @content = lambda { |chars| @currentKb.layout.spacing = chars.to_f * Scale }
      when "xi:include", "include"
        filename = attr_hash['href']
        self.include(filename)
      when "Colors"
        @currentKb.colors = {}
        @state.push :Colors
      when "Mount"
        @currentKb.layout.mount = Size.new(0.0, 0.0)
        @state.push :Mount
      when "KeyDefs"
        @state.push :KeyDefs
      else
        raise UnknownElementError.new(name, @state, attr_hash, @filenames)
      end
    when :Map
      case name
      when "Keys"
        if attr_hash.has_key? 'default_color'
          defaultColor = @currentKb.colors[attr_hash['default_color']]
        end
        @state.push :Keys
      else
        raise UnknownElementError.new(name, @state, attr_hash, @filenames)
      end
    when :Matrix
      case name
      when "Row"
        @currentMatrixRow = attr_hash['id'].to_i
        @currentKb.matrix.push Array.new
        @state.push :MatrixRow
      else
        raise UnknownElementError.new(name, @state, attr_hash, @filenames)
      end

    # <Matrix> sub-elements
    when :MatrixRow
      case name
      when "Col"
        matrixCol = attr_hash['id'].to_i
        @currentKb.matrix[@currentMatrixRow][matrixCol] = nil
        @content = lambda { |chars| @currentKb.matrix[@currentMatrixRow][matrixCol] = chars;
                                    @currentKb.reverseMatrix[chars] = Cell.new(@currentMatrixRow, matrixCol)}
      else
        raise UnknownElementError.new(name, @state, attr_hash, @filenames)
      end

    # <Layout> sub-elements
    when :Colors
      case name
      when "Color"
        @state.push :Color
        @currentColor = attr_hash['name']
      else
        raise UnknownElementError.new(name, @state, attr_hash, @filenames)
      end
    when :Mount
      case name
      when "Height"
        @content = lambda { |chars| @currentKb.layout.mount.height = chars.to_f * Scale }
      when "Width"
        @content = lambda { |chars| @currentKb.layout.mount.width = chars.to_f * Scale }
      else
        raise UnknownElementError.new(name, @state, attr_hash, @filenames)
      end
    when :KeyDefs
      case name
      when "Row"
        @pos.x = 0.0
        @currentRow += 1
        @currentRowDef = RowDef.new
        @state.push :KeyDefRow
      else
        raise UnknownElementError.new(name, @state, attr_hash, @filenames)
      end

    # <KeyDefs> sub-elements
    when :KeyDefRow
      case name
      when "OffsetY"
        @content = lambda { |chars|
                           @currentRowDef.offset.y = chars.to_f * Scale
                           @pos.y += @currentRowDef.offset.y + @currentKb.layout.spacing }
      when "OffsetX"
        @content = lambda { |chars|
                           @currentRowDef.offset.x = chars.to_f * Scale
                           @pos.x += @currentRowDef.offset.x }
      when "KeyDef"
        location = attr_hash['location']
        @currentKeyDef = KeyDef.new(location)
        @currentKb.keyhash[location] = @currentKeyDef
        @currentKeyDef.origin = @pos.clone
        @state.push :KeyDef
      else
        raise UnknownElementError.new(name, @state, attr_hash, @filenames)
      end

    # <Row> (of <KeyDef>) sub-elements
    when :KeyDef
      case name
      when "Height"
        @content = lambda { |chars|
                           @currentKeyDef.size.height = chars.to_f * Scale }
      when "Width"
        @content = lambda { |chars|
                           @currentKeyDef.size.width = chars.to_f * Scale
                           @pos.x += @currentKeyDef.size.width + @currentKb.layout.spacing }
      when "OffsetY"
        @content = lambda { |chars|
                           @currentKeyDef.offset.y = chars.to_f * Scale
                           @currentKeyDef.origin.y = @pos.y + @currentKeyDef.offset.y }
      when "OffsetX"
        @content = lambda { |chars|
                           @currentKeyDef.offset.x = chars.to_f * Scale
                           @currentKeyDef.origin.x = @pos.x + @currentKeyDef.offset.x }
      when "Background"
        @content = lambda { |chars| @currentKeyDef.background = @currentKb.colors[chars] }
      when "Bump"
        @currentKeyDef.bump = true
      when "NullKey"
        # ignore
      else
        raise UnknownElementError.new(name, @state, attr_hash, @filenames)
      end

    # <Colors> sub-elements
    when :Color
      case name
      when "Rgb"
        @content = lambda { |chars| @currentKb.colors[@currentColor] = chars  }
      else
        raise UnknownElementError.new(name, @state, attr_hash, @filenames)
      end

    # <Map> sub-elements
    when :Keys
      case name
      when "Key"
        location = attr_hash['location']
        if !@currentKb.keyhash.has_key? location
          raise "ERROR: Key definition #{location} has not been defined"
        end
        if @currentKb.keyhash[location].null
          raise "ERROR: Cannot assign key to a Null key definition (#{location})"
        end
        oldKey = nil
        if @currentKeyMap.keys.has_key? location
          oldKey = @currentKeyMap.keys[location]
        end
        @currentKey = @currentKeyMap.keys[location] = Key.new(location, defaultColor)
        @currentKey.prevKey = oldKey
        @state.push :Key
      else
        raise UnknownElementError.new(name, @state, attr_hash, @filenames)
      end

    # <Keys> sub-elements
    when :Key
      case name
      when "Label"
        @state.push :Label
      when "Usage"
        @currentKey.usage = process_usage(@currentKey.location, attr_hash)
      when "MacroKey"
        @currentKey.macro.push MacroKey.new
        @state.push :MacroKey
      when "Macro"
        @state.push :Macro
      when "ModeKey"
        @currentKey.modeMapID = attr_hash["map-id"]
        @currentKey.modeType = attr_hash["type"]
        @currentKey.modeLED = attr_hash["led"]
        @content = lambda { |chars| @currentKey.mode += chars }
      else
        raise UnknownElementError.new(name, @state, attr_hash, @filenames)
      end

    # <Macro> sub-elements
    when :Macro
      case name
        when "MacroKey"
        @currentKey.macro.push MacroKey.new
        @state.push :MacroKey
      end

    # <Key> sub-elements
    when :Label
      legendColor = @currentKb.colors[attr_hash['color']] if attr_hash.has_key? 'color'
      note = attr_hash.has_key?('note') ? attr_hash['note'] : nil
      case name
      when "TopLeft"
        @currentKey.label.topLeft = KeyLegend.new( '', legendColor, note )
        @content = lambda { |chars| @currentKey.label.topLeft.text = chars }
      when "TopCenter"
        @currentKey.label.topCenter = KeyLegend.new( '', legendColor, note )
        @content = lambda { |chars| @currentKey.label.topCenter.text = chars }
      when "TopRight"
        @currentKey.label.topRight = KeyLegend.new( '', legendColor, note )
        @content = lambda { |chars| @currentKey.label.topRight.text = chars }
      when "BottomLeft"
        @currentKey.label.bottomLeft = KeyLegend.new( '', legendColor, note )
        @content = lambda { |chars| @currentKey.label.bottomLeft.text = chars }
      when "BottomCenter"
        @currentKey.label.bottomCenter = KeyLegend.new( '', legendColor, note )
        @content = lambda { |chars| @currentKey.label.bottomCenter.text = chars }
      when "BottomRight"
        @currentKey.label.bottomRight = KeyLegend.new( '', legendColor, note )
        @content = lambda { |chars| @currentKey.label.bottomRight.text = chars }
      when "Center"
        @currentKey.label.center = KeyLegend.new( '', legendColor, note )
        @content = lambda { |chars| @currentKey.label.center.text = chars }
      else
        raise UnknownElementError.new(name, @state, attr_hash, @filenames)
      end
    when :MacroKey
      case name
      when "Modifier"
        @currentKey.macro.last.modifiers |= process_modifier(attr_hash)
      when "Usage"
        if @currentKey.macro.last.usage != nil
          raise "ERROR: multiple usages specified for macro key at #{@currentKey.location}"
        end
        @currentKey.macro.last.usage = process_usage(@currentKey.location, attr_hash)
      else
        raise UnknownElementError.new(name, @state, attr_hash, @filenames)
      end
    else
      raise "ERROR: Unknown element '#{name}'"
    end
  end

  def on_characters(chars)
    if @content != nil
       @content.call( chars )
    end
  end

  def on_end_element(name)
    @level -= 1

    case @state.last

    # Root elements
    when :Root
    when :Copyright
      case name
      when "Copyright"
        @copyright.strip!
        @state.pop
        @content = nil
      end
    when :Keyboard, :Keyboards
      case name
      when "Keyboard"
        # @currentKb.layout is now cooked, so freeze it
        @currentKb.layout.freeze
        @currentKb = nil
        @state.pop
      end

    # <Keyboard> sub-elements
    when :Layout
      case name
      when "Layout"
        @state.pop
      when "Spacing"
        @content = nil
      end
    when :Map
      case name
      when "Map"
        @currentKeyMap = nil
        @state.pop
      end
    when :Matrix
      case name
      when "Matrix"
        @state.pop
      end

    # <Matrix> sub-elements
    when :MatrixRow
      case name
      when "Col"
        @content = nil
      when "Row"
        @state.pop
      end

    # <Layout> sub-elements
    when :Colors
      case name
      when "Colors"
        @state.pop
      end
    when :Mount
      case name
      when "Height", "Width"
        @content = nil
      when "Mount"
        @state.pop
      end
    when :KeyDefs
      case name
      when "KeyDefs"
        @state.pop
      end

    # <KeyDefs> sub-elements
    when :KeyDefRow
      case name
      when "Row"
        @currentKb.layout.rows[@currentRow] = @currentRowDef
        @currentRowDef = nil
        @state.pop
      when "OffsetX", "OffsetY"
        @content = nil
      end

    # <Row> (of <KeyDef>) sub-elements
    when :KeyDef
      case name
      when "KeyDef"
        @currentKb.max.x = max?(@currentKb.max.x, @currentKeyDef.origin.x + @currentKeyDef.size.width)
        @currentKb.max.y = max?(@currentKb.max.y, @currentKeyDef.origin.y + @currentKeyDef.size.height)
        @currentRowDef.keydefs.push @currentKeyDef
        @currentKeyDef = nil
        @state.pop
      when "Height", "Width", "OffsetX", "OffsetY", "Background"
        @content = nil
      when "NullKey"
        @currentKeyDef.null = true
      end

    # <Colors> sub-elements
    when :Color
      case name
      when "Color"
        @currentColor = nil
        @state.pop
      when "Rgb"
        @content = nil
      end

    # <Map> sub-elements
    when :Keys
      case name
      when "Keys"
        @state.pop
      end

    # <Keys> sub-elements
    when :Key
      case name
      when "Key"
        @currentKey = nil
        @state.pop
      when "Usage"
        # do nothing, already handled
      when "ModeKey"
         @content = nil
      end

    # <Key> sub-elements
    when :Label
      case name
      when "Label"
        @state.pop
      when "Center", "TopLeft", "TopCenter", "TopRight", "BottomLeft", "BottomCenter", "BottomRight"
        @content = nil
      end
    when :Macro
      case name
      when "Macro"
        @state.pop
      end
    when :MacroKey
      case name
      when "MacroKey"
        @state.pop
      when "Usage"
        # do nothing, already handled
      end
    end
  end

  def on_end_document

  end

end
