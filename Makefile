all: 
	gcc -g -Wall -o s-talk -pthread UDPthreads.c UDPsockets.c list.c main.c

main.o: main.c threads.h
	gcc -g -Wall -c main.c

UDPthreads.o: threads.c threads.h
	gcc -g -Wall -c threads.c

UDPsockets.o: sockets.c sockets.h
	gcc -g -Wall -c sockets.c

list.o: list.c list.h
	gcc -g -Wall -c list.c

clean:
	rm s-talk