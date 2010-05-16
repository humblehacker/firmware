#ifndef __HIDUSAGETABLES_H__
#define __HIDUSAGETABLES_H__

#define MAKE_USAGE(page,id) (((page)<<10)|(id))
#define USAGE_ID(usage)     (usage&~(0xffff<<10))
#define USAGE_PAGE(usage)   (usage>>10)

#define HID_USAGE_NONE        0UL
#define HID_USAGE_PAGE_CUSTOM 0x3f
#define HID_USAGE_CUSTOM      MAKE_USAGE(HID_USAGE_PAGE_CUSTOM,0)
#define HID_USAGE_PAGE_MACRO  0x3e
#define HID_USAGE_MACRO(x)    MAKE_USAGE(HID_USAGE_PAGE_MACRO,(x))

typedef uint8_t  UsagePage;
typedef uint8_t  UsageID;
typedef enum
{
<% $hid.pages.each do |usagePage| %>
  /* USAGE PAGE:  <%=usagePage.name %> */
#define HID_USAGE_PAGE_<%= normalize_identifier(usagePage.name)%> <%=dec_to_hex(usagePage.id)%>

<%   $hid.usagesByPage[usagePage].each do |usage| %>
  HID_USAGE_<%=normalize_identifier(usage.name)%> = MAKE_USAGE(<%=dec_to_hex(usagePage.id)%>, <%=dec_to_hex(usage.id)%>),
<%   end %>
<% end %>

} Usage;

#endif /* __HIDUSAGETABLES_H__ */

