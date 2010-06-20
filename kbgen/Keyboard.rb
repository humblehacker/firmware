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

require 'xml/libxml'
require 'shell'
require 'KbInternals.rb'
require 'Utils.rb'

class Keyboard
  attr_accessor :layout, :maps, :colors, :org, :keyhash, :max,
                :copyright, :matrix, :matrixId, :reverseMatrix,
                :cports, :rports, :defaultMap
  Scale = 28.34627813368
  def initialize(org="none")
    @org = org
    @layout = nil
    @maps   = {}
    @keyhash = {}
    @matrix = []
    @reverseMatrix = {}
    @matrixId = nil
    @defaultMap = nil
    @colors = nil
    @copyright = ''
    @max = Point.new(0,0)
    @cports = []
    @rports = []
  end

  def key_def_from_ID(id)
    return nil if ! @keyhash.has_key? id
    return @keyhash[id]
  end

end
