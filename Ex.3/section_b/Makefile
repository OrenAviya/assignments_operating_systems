# project/section_b/Makefile
all: libproactor.a

libproactor.a: proactor.o
	ar rcs libproactor.a proactor.o

proactor.o: proactor.c proactor.h
	gcc -c -o proactor.o proactor.c -pthread

clean:
	rm -f libproactor.a proactor.o
