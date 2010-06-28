#include <stdio.h>
#include <assert.h>

#include "report_queue.c"

int main (int argc, char *argv[])
{
  // Preliminaries
  assert(sizeof(uint8_t) == 1);
  assert(sizeof(uint16_t) == 2);

  // Initialization
  ReportQueue__init();
  assert(ReportQueue__freespace() == 10);
  assert(ReportQueue__is_empty() == true);
  assert(ReportQueue__is_full() == false);
  assert(self.data_count == MAX_REPORT_QUEUE);
  assert(self.count == 0);
  assert(self.front == 0);

  // add one element
  RQ_elem_t *elem = ReportQueue__push();
  RQ_elem_t *first = elem;
  assert(elem != NULL);
  assert(ReportQueue__freespace() == 9);
  assert(ReportQueue__is_empty() == false);
  assert(ReportQueue__is_full() == false);
  assert(self.count == 1);
  assert(self.front == 0);

  // add eight more
  for (int x = 8; x > 0; --x)
  {
    ReportQueue__push();
    assert(elem != NULL);
    assert(ReportQueue__freespace() == x);
    assert(ReportQueue__is_empty() == false);
    assert(ReportQueue__is_full() == false);
  }
  assert(self.count == 9);
  assert(self.front == 0);

  // one more to fill it up
  elem = ReportQueue__push();
  assert(elem != NULL);
  assert(ReportQueue__freespace() == 0);
  assert(ReportQueue__is_empty() == false);
  assert(ReportQueue__is_full() == true);
  assert(self.count == 10);
  assert(self.front == 0);

  // try to push one more
  elem = ReportQueue__push();
  assert(elem == NULL);
  assert(ReportQueue__freespace() == 0);
  assert(ReportQueue__is_empty() == false);
  assert(ReportQueue__is_full() == true);
  assert(self.count == 10);
  assert(self.front == 0);

  // peek at the first one
  elem = ReportQueue__peek();
  assert(elem == first);
  assert(ReportQueue__freespace() == 0);
  assert(ReportQueue__is_empty() == false);
  assert(ReportQueue__is_full() == true);

  // pop the first one
  elem = ReportQueue__pop();
  assert(elem == first);
  assert(ReportQueue__freespace() == 1);
  assert(ReportQueue__is_empty() == false);
  assert(ReportQueue__is_full() == false);
  assert(self.count == 9);
  assert(self.front == 1);

  first = ReportQueue__peek();

  // push another one, first wrap-around
  elem = ReportQueue__push();
  assert(elem != NULL);
  assert(ReportQueue__freespace() == 0);
  assert(ReportQueue__is_empty() == false);
  assert(ReportQueue__is_full() == true);
  assert(self.count == 10);
  assert(self.front == 1);

  // pop all 10 elements
  for (int x = 0; x < 10; ++x)
  {
    ReportQueue__pop();
  }
  assert(ReportQueue__freespace() == 10);
  assert(ReportQueue__is_empty() == true);
  assert(ReportQueue__is_full() == false);
  assert(self.data_count == MAX_REPORT_QUEUE);
  assert(self.count == 0);
  assert(self.front == 1);

  printf("Success!\n");
}
