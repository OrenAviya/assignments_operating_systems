CC = gcc
CFLAGS = -Wall -Wextra -pthread
INCLUDES = -I section_b

all: chat_section_a section_c/server_proactor

chat_section_a:
	@$(MAKE) -C section_a

section_c/server_proactor: section_c/server_proactor.c section_b/libproactor.a
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@ -Lsection_b -lproactor


section_b/libproactor.a: section_b/proactor.o
	ar rcs $@ $^

section_b/proactor.o: section_b/proactor.c section_b/proactor.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f section_c/server_proactor section_b/proactor.o section_b/libproactor.a
	@$(MAKE) -C section_a clean
