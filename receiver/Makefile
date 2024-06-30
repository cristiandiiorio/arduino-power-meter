CC = gcc
CFLAGS = -Wall -Wextra

receiver: receiver.o misc.o
	$(CC) $(CFLAGS) -o receiver receiver.o misc.o

receiver.o: receiver.c receiver.h
	$(CC) $(CFLAGS) -c receiver.c

misc.o: misc.c
	$(CC) $(CFLAGS) -c misc.c

clean:
	rm -f receiver receiver.o misc.o