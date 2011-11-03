TARGET = s301
CC = gcc
CFLAGS += -Wall -Wextra -Werror -static

SRC = $(wildcard *.c)
OBJ = $(SRC:%.c=%.o)

all: $(TARGET)
$(TARGET): $(OBJ)
	$(CC) $^ $(CFLAGS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

test: $(TARGET) req
	valgrind --tool=memcheck --leak-check=yes --track-origins=yes ./$(TARGET) < req

clean:
	-rm -f $(OBJ) $(TARGET)

.PHONY: all clean test
