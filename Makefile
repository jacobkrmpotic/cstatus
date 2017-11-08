CC = gcc
SRC = cstatus.c
FLAGS = -Wall -pedantic -lX11
EXEC = cstatus
DEP = /usr/include/X11/Xlib.h

build: $(SRC) $(DEP)
	$(CC) $(FLAGS) $(SRC) -o $(EXEC)

clean: $(EXEC)
	rm $(EXEC)

