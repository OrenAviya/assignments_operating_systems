all: libencriptor encode decode

decode:	encriptor.h decode.c libencriptor
	gcc decode.c -L. -lencriptor -o decode

encode:	encriptor.h encode.c libencriptor
	gcc encode.c -L. -lencriptor -o encode

libencriptor:	encriptor.h encriptor.c
	gcc -shared encriptor.c -fPIC -o libencriptor.so

.PHONY: clean
clean:
	-rm decode encode libencriptor.so
