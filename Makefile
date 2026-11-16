CC = gcc
CFLAGS = -Wall -Wextra -Werror -Wpedantic -std=c99
TARGET = main

$(TARGET): main.c
	$(CC) $(CFLAGS) main.c -o $(TARGET)

clean:
	rm -f $(TARGET)
