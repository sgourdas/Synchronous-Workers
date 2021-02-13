HEADER  = Types.h
SOURCE	= chef.c saladmakers.c
OBJS 	= chef.o saladmakers.o
EXECS   = chef saladmakers
LOGS   = saladmaker1.log saladmaker2.log saladmaker3.log unified.log
OUT  	= chef saladmakers
CC	    = gcc
CFLAGS  = -g -Wall -Werror -pthread


make: chef.o saladmakers.o
	$(CC) $(CFLAGS) chef.o -o chef
	$(CC) $(CFLAGS) saladmakers.o -o saladmakers

run: 
	./chef -n 10 -m 2

debug: 
	valgrind ./chef -n 4 -m 3

chef.o: chef.c
	$(CC) -c $(CFLAGS) chef.c

saladmakers.o: saladmakers.c
	$(CC) -c $(CFLAGS) saladmakers.c

clean:
	rm -f $(OBJS) $(EXECS) $(LOGS)