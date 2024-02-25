# Makefile
all: proactor_lib libserver

proactor_lib:
	$(MAKE) -C proactor_lib

libserver: server_proactor.o
	gcc -o libserver server_proactor.o -Lproactor_lib -lproactor -pthread

server_proactor.o: server_proactor.c
	gcc -c -o server_proactor.o server_proactor.c -Iproactor_lib

clean:
	$(MAKE) -C proactor_lib clean
	rm -f libserver server_proactor.o
