CFLAGS = -std=c11 -Wall -Wextra -Werror -g -O0 
all: test_list

list.o: list.c list.h
	gcc $(CFLAGS) -c list.c -o 	list.o

test_list.o: test_list.c list.h
	gcc $(CFLAGS) -c test_list.c -o test_list.o

test_list: list.o test_list.o 
	gcc list.o test_list.o -o test_list	

.PHONY: clean

clean:
	rm -f list.o test_list.o test_list

