all: test_report_queue
	./test_report_queue

test_report_queue: test_report_queue.o
	gcc -o $@ $^

%.o : %.c
	gcc -g -std=gnu99 -c -o $@ $^ -D TEST_REPORT_QUEUE

clean:
	@rm -f *.o
	@rm -f ./test_report_queue

