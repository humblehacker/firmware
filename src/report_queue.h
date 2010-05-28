#ifndef __REPORT_QUEUE_H__
#define __REPORT_QUEUE_H__

#include "keyboard_report.h"

#include <LUFA/Drivers/USB/Class/HID.h>

typedef KeyboardReport RQ_elem_t;

void       ReportQueue__init(void);

           // returns the recently pushed report, or NULL if queue is full.
RQ_elem_t* ReportQueue__push(void);

           // returns the recently removed report, or NULL if queue is empty.
RQ_elem_t* ReportQueue__pop(void);

           // returns most recent report, or NULL if queue is empty.
RQ_elem_t* ReportQueue__peek(void);

           // return true if queue is empty.
bool       ReportQueue__is_empty(void);

           // return true if queue is full
bool       ReportQueue__is_full(void);



#endif // __REPORT_QUEUE_H__
