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

#ifndef __REPORT_QUEUE_H__
#define __REPORT_QUEUE_H__

#include <stdbool.h>

#if defined(TEST_REPORT_QUEUE)

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef uint16_t RQ_elem_t;

#else

#include "keyboard_report.h"
#include <LUFA/Drivers/USB/Class/HID.h>
typedef KeyboardReport RQ_elem_t;

#endif

void       ReportQueue__init(void);

           // returns the recently pushed report, or NULL if queue is full.
RQ_elem_t* ReportQueue__push(void);

           // returns the recently removed report, or NULL if queue is empty.
           // NOTE: the returned report is only guaranteed to be valid until
           //       the next push.
RQ_elem_t* ReportQueue__pop(void);

           // returns most recent report, or NULL if queue is empty.
RQ_elem_t* ReportQueue__peek(void);

           // return true if queue is empty.
bool       ReportQueue__is_empty(void);

           // return true if queue is full.
bool       ReportQueue__is_full(void);

           // return the number of remaining free slots in queue.
uint8_t    ReportQueue__freespace(void);


#endif // __REPORT_QUEUE_H__
