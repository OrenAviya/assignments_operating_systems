CC = gcc
CFLAGS_DEBUG = -g
CFLAGS_RELEASE = -O2

SRC_DIVISION = divisionByZero.c
SRC_RECURSIVE = infiniteRecursive.c

EXE_DEBUG_DIVISION = program_with_debug_division
EXE_RELEASE_DIVISION = program_without_debug_division

EXE_DEBUG_RECURSIVE = program_with_debug_recursive
EXE_RELEASE_RECURSIVE = program_without_debug_recursive

.PHONY: all debug release clean debug_gdb release_gdb

all: debug release

debug: debug_division debug_recursive

release: release_division release_recursive

debug_division: $(SRC_DIVISION)
	$(CC) $(CFLAGS_DEBUG) -o $(EXE_DEBUG_DIVISION) $(SRC_DIVISION)

release_division: $(SRC_DIVISION)
	$(CC) $(CFLAGS_RELEASE) -o $(EXE_RELEASE_DIVISION) $(SRC_DIVISION)

debug_recursive: $(SRC_RECURSIVE)
	$(CC) $(CFLAGS_DEBUG) -o $(EXE_DEBUG_RECURSIVE) $(SRC_RECURSIVE)

release_recursive: $(SRC_RECURSIVE)
	$(CC) $(CFLAGS_RELEASE) -o $(EXE_RELEASE_RECURSIVE) $(SRC_RECURSIVE)

clean:
	rm -f $(EXE_DEBUG_DIVISION) $(EXE_RELEASE_DIVISION) $(EXE_DEBUG_RECURSIVE) $(EXE_RELEASE_RECURSIVE)

debug_gdb: debug_division debug_recursive
	gdb ./$(EXE_DEBUG_DIVISION)

release_gdb: release_division release_recursive
	gdb ./$(EXE_RELEASE_DIVISION)
