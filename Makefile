CC=gcc
CFLAGS=-g -pedantic -std=gnu17 -Wall -Werror -Wextra

.PHONY: all
all: nyush

nyush: nyush.o read_command.o prompt.o run_command.o utility.o

nyush.o: nyush.c read_command.h prompt.h run_command.h utility.h

read_command.o: read_command.c read_command.h

prompt.o: prompt.c prompt.h

run_command.o : run_command.c run_command.h

utility.o : utility.c utility.h

.PHONY: clean
clean:
	rm -f *.o nyush
