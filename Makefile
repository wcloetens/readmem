CROSSCOMPILE =

CC = $(CROSSCOMPILE)gcc
STRIP = $(CROSSCOMPILE)strip

CFLAGS = -Wall -W -static

readmem: readmem.c
	$(CC) $(CFLAGS) -o $@ $^
	$(STRIP) $@

clean:
	rm -f readmem

.PHONY: clean
