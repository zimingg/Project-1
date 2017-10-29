CFLAGS=-W -Wall -g
PROGRAMS= part1 part2 part3 part4

all: $(PROGRAMS)


part1: p1fxns.o th1.o
	gcc $(CFLAGS) -o part1 $^

part2: p1fxns.o th2.o
	gcc $(CFLAGS) -o part2 $^

part3: p1fxns.o th3.o taskqueue.o
	gcc $(CFLAGS) -o part3 $^

part4: p1fxns.o th4.o taskqueue.o
	gcc $(CFLAGS) -o part4 $^

clean:
	rm -f *.o $(PROGRAMS)


p1fxns.o: p1fxns.c p1fxns.h
taskqueue.o: taskqueue.c taskqueue.h
th1.o: th1.c
th2.o: th2.c
th3.o: th3.c
th4.o: th4.c
