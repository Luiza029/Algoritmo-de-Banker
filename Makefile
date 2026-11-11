banker: main.o banker.o
	gcc main.o banker.o -o banker

main.o: main.c banker.h
	gcc -Wall -Wextra -c main.c

banker.o: banker.c banker.h
	gcc -Wall -Wextra -c banker.c

run: banker
	./banker

clean:
	rm -f *.o banker result.txt

.PHONY: run clean