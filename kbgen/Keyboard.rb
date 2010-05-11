require 'xml/libxml'
require 'shell'
require 'KbInternals.rb'
require 'Utils.rb'

class Keyboard
  attr_accessor :layout, :maps, :colors, :org, :keyhash, :max, 
                :copyright, :matrix, :matrixId, :reverseMatrix,
                :defaultMap
  Scale = 28.34627813368
  def initialize(org)
    @org = org
    @layout = nil
    @maps   = []
    @keyhash = {}
    @matrix = []
    @reverseMatrix = {}
    @matrixId = nil
    @defaultMap = nil
    @colors = nil
    @copyright = ''
    @max = Point.new(0,0)
  end

  def key_def_from_ID(id)
    return nil if ! @keyhash.has_key? id
    return @keyhash[id]
  end
  
end
