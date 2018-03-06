CC = gcc
PFLAGS = -g -Wall
SOURCE = src/wyd.c
LIBS = -L /usr/bin/sqlite3 -lm -lsqlite3
INCLUDES = -I include -I /usr/include/sqlite3

all: $(SOURCE)
	$(CC) $(PFLAGS) $(SOURCE) -o wyd $(LIBS) $(INCLUDES)

install:
	sudo cp wyd /usr/bin/

.PHONY: clean
clean:
	sudo rm -f /usr/bin/wyd wyd ~/.WYD.db