all:	bash

bash: functions.o shell.o functions.h
	gcc -g functions.o shell.o -lrt

functions.o: functions.c functions.h
	gcc -g -c functions.c -lrt

shell.o:	shell.c functions.h
	gcc -g -c shell.c -lrt

clean:
	rm *.o a.out
