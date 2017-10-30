CFLAGS=-W -Wall -g
PROGRAMS= thv1 thv2 thv3 thv4

all: $(PROGRAMS)


thv1: p1fxns.o thv1.o
	gcc $(CFLAGS) -o thv1 $^

thv2: p1fxns.o thv2.o
	gcc $(CFLAGS) -o thv2 $^

thv3: p1fxns.o thv3.o taskqueue.o
	gcc $(CFLAGS) -o thv3 $^

thv4: p1fxns.o thv4.o taskqueue.o
	gcc $(CFLAGS) -o thv4 $^

clean:
	rm -f *.o $(PROGRAMS)


p1fxns.o: p1fxns.c p1fxns.h
taskqueue.o: taskqueue.c taskqueue.h
thv1.o: thv1.c
thv2.o: thv2.c
thv3.o: thv3.c
thv4.o: thv4.c
