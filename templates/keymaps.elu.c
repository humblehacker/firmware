/*
                    The HumbleHacker Keyboard Project
                 Copyright © 2008-2010, David Whetstone
               david DOT whetstone AT humblehacker DOT com

  This file is a part of The HumbleHacker Keyboard Project.

  The HumbleHacker Keyboard Project is free software: you can redistribute
  it and/or modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  The HumbleHacker Keyboard Project is distributed in the hope that it will
  be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
  Public License for more details.

  You should have received a copy of the GNU General Public License along
  with The HumbleHacker Keyboard Project.  If not, see
  <http://www.gnu.org/licenses/>.

*/

#include "keymaps.h"
#include "hid_usages.h"
<%
   flipped_matrix = {}
   for irow,row in ipairs(kb.matrix.rows) do
     for icol,location in ipairs(row) do
       if flipped_matrix[icol] == nil then
         flipped_matrix[icol] = {}
       end
       flipped_matrix[icol][irow] = location
     end
   end

   for mapname,keymap in pairs(kb.keymaps) do %>
const KeyBindingArray keymap_<%=keymap.name%>[] =
{
<%   lastcol = #flipped_matrix
     for icol,col in ipairs(flipped_matrix) do
%>
  /* col: <%=icol-1%> */<%
       last_row = 0
       for irow,location in ipairs(col) do
         last_row = irow %>
  /* row:<%=irow-1%> loc:<%=location%> */ <%
         relevant_keymap, key = lookup_key(keymap, location)
         if key == nil then %>{0, NULL}<%
         elseif #key.bindings == 0 then %>{0, NULL} /* EMPTY DEFINITION! */<%
         else %>{<%=#key.bindings%>, &<%=relevant_keymap.name%>_<%=key.location%>[0]}<%
         end %>,<%
       end
       for i=(last_row+1),8 do %>
  /* ---:<%=i-1%> loc:-- */ {0, NULL},<%
       end %>
<%   end %>
};

<%   if keymap.is_default then %>
const KeyMap default_keymap = &keymap_<%=keymap.name%>[0];
<%   end
   end
%>
