
all:
	gcc -g -Wall -o s-talk -pthread listMonitor.c main.c list.c
	gcc -g -Wall -o testMonitor -pthread listMonitor.c listMonitorTest.c list.c 
# 			^^ Uncomment to test listMonitor ^^ 

main.o: main.c listMonitor.h
	gcc -g -Wall -c main.c

listMonitor.o: 	listMonitor.c listMonitor.h
	gcc -g -Wall -c listMonitor.c

list.o: 	list.c list.h
	gcc -g -Wall -c list.c

clean:
	rm s-talk
	rm testMonitor