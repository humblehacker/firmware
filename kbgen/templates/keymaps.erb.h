#ifndef __KEYMAPS_H__
#define __KEYMAPS_H__

<% $keymapIDs.each do |name, id| %>
#include "<%=id%>_mx.h"
<% end %>

#endif // __KEYMAPS_H__
