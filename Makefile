CC = gcc
CFLAGS = -Wall -Wextra -g

conway: src/main.c
	$(CC) $(CFLAGS) src/main.c -o conway

clean:
	rm -f conway
