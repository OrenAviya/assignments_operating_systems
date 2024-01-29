# List of subdirectories containing code and Makefile
SUBDIRS := debugExample Polyalphabetic_encoder PythagoreanTtiangle TarCompressEncrypt

# Default target
all: $(SUBDIRS)

# Recursively execute make in each subdirectory
$(SUBDIRS):
	$(MAKE) -C $@

.PHONY: all $(SUBDIRS)

# Clean up generated files
clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean;\
	done