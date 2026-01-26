# Ryft - Markdown code block extractor
VERSION = 0.1.0

CC ?= cc
CFLAGS ?= -Wall -Wextra -pedantic -std=c99 -O2
CFLAGS += -I. -DRYFT_VERSION=\"$(VERSION)\"

PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin

SRCDIR = src
BINDIR_LOCAL = bin
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:.c=.o)
TARGET = $(BINDIR_LOCAL)/ryft

$(TARGET): $(OBJECTS) | $(BINDIR_LOCAL)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)

$(BINDIR_LOCAL):
	mkdir -p $(BINDIR_LOCAL)

$(SRCDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) $(SRCDIR)/*.o

install: $(TARGET)
	install -d $(DESTDIR)$(BINDIR)
	install -m 755 $(TARGET) $(DESTDIR)$(BINDIR)/ryft

.PHONY: clean install
