banker: banker.o
	gcc banker.o -o banker

banker.o: banker.c
	gcc -Wall -Wextra -c banker.c

run: banker
	./banker

clean:
	rm -f *.o banker result.txt

.PHONY: run clean