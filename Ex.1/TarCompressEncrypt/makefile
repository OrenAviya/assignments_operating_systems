CC := gcc
CFLAGS := -Wall -Wextra -std=c99

all: zip unzip

zip: myzip.c
	$(CC) $(CFLAGS) myzip.c -o myzip

unzip: myunzip.c
	$(CC) $(CFLAGS) myunzip.c -o myunzip

clean:
	rm -f myzip myunzip
