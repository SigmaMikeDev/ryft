# Ryft - Markdown code block extractor
CC ?= cc
CFLAGS ?= -Wall -Wextra -pedantic -std=c99 -O2

PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin

ryft: ryft.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f ryft

install: ryft
	install -d $(DESTDIR)$(BINDIR)
	install -m 755 ryft $(DESTDIR)$(BINDIR)/

.PHONY: clean install
