CC = gcc
CFLAGS = -Wall -Wextra -std=c11
LDFLAGS = -lm

TARGET = pythagoreanTriangle
SRC = pythagoreanTriangle.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
