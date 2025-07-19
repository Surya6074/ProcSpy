CC = gcc
CFLAGS = -Wall -Wextra -Iinclude
SRC = src/main.c
OUT = procspy
LIBS = -lncurses

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(OUT) $(LIBS)

clean:
	rm -f $(OUT)