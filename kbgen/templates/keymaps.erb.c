#include "keymaps.h"

<% $keymapIDs.each do |name, id| %>
#include "<%=id%>_mx.c"
<% end %>

