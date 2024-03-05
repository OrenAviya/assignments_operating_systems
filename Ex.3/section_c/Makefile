# project/section_c/Makefile
all: libserver

libserver: server_proactor.o
	gcc -o libserver server_proactor.o -L../section_b -lproactor -pthread

server_proactor.o: server_proactor.c
	gcc -c -o server_proactor.o server_proactor.c -I../section_b

clean:
	rm -f libserver server_proactor.o