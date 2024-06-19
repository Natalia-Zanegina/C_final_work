all: drone

drone: drone.o functions.o
	gcc -o drone drone.o functions.o -lncurses

drone.o: drone.c
	gcc -c -o drone.o drone.c -lncurses

functions.o: functions.c
	gcc -c -o functions.o functions.c -lncurses

clean:
	rm -rf *.o drone