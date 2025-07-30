CC = gcc
CFLAGS = -Wall -Wextra -Iinclude
SRC = src/main.c src/parser.c src/utils/utils.c src/data.c src/ui.c src/server.c
OUT = procspy
LIBS = -lncurses

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(OUT) $(LIBS)

clean:
	rm -f $(OUT)