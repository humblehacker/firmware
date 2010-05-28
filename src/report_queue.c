#include <stdio.h>
#include <stdlib.h>
#include "report_queue.h"

struct ReportQueue
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
    return false;

  self.count++;
  self.rear = (self.rear + 1) % self.data_size;
  KeyboardReport__init(&self.data[self.rear]);
  memset(&self.data[self.rear], 0, sizeof(RQ_elem_t));
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

bool
ReportQueue__is_empty()
{
  return self.count <= 0;
}

bool
ReportQueue__is_full()
{
  return self.count >= self.data_size;
}

