CC = gcc

all: scheduler enq deq stat

scheduler: heap.o	
	$(CC) -o scheduler scheduler.c heap.o

heap.o:

enq:
	$(CC) -o enq enq.c

deq:
	$(CC) -o deq deq.c

stat:
	$(CC) -o stat stat.c

debug: CC += -g
debug: scheduler enq deq stat

clean:
	rm enq scheduler deq stat
