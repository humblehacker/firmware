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

#include <stdio.h>
#include <stdlib.h>
#include "report_queue.h"

static struct ReportQueue
{
  uint8_t   front;
  uint8_t   rear;
  uint8_t   count;
  RQ_elem_t data[100];
  uint8_t   data_size;
} self;

void
ReportQueue__init()
{
  self.front     = 0;
  self.rear      = 0;
  self.count     = 0;
  self.data_size = sizeof(self.data)/sizeof(self.data[0]);
}

RQ_elem_t *
ReportQueue__push()
{
  if (ReportQueue__is_full())
    return NULL;

  self.count++;
  self.rear = (self.rear + 1) % self.data_size;
  KeyboardReport__init(&self.data[self.rear]);
  return &self.data[self.rear];
}

RQ_elem_t *
ReportQueue__pop()
{
  if (ReportQueue__is_empty())
    return NULL;

  RQ_elem_t *result = &self.data[self.front];
  self.front = (self.front + 1) % self.data_size;
  self.count--;
  return result;
}

RQ_elem_t  *
ReportQueue__peek()
{
  if (ReportQueue__is_empty())
    return NULL;

  return &self.data[self.front];
}

RQ_elem_t  *
ReportQueue__prev()
{
  if (ReportQueue__is_empty())
    return NULL;

  uint8_t prev_front = (self.data_size + self.front - 1) % self.data_size;
  if (prev_front == self.rear)
    return NULL;

  return &self.data[prev_front];
}

bool
ReportQueue__is_empty()
{
  return self.count <= 0;
}

uint8_t
ReportQueue__freespace()
{
  return self.data_size - self.count;
}

bool
ReportQueue__is_full()
{
  return self.count >= self.data_size;
}

