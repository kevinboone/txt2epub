VERSION := 0.0.6
CC      := gcc
LIBS    := -lpcre
DESTDIR ?= /
PREFIX  ?= /usr
MANDIR  := $(DESTDIR)/$(PREFIX)/share/man
BINDIR  := $(DESTDIR)/$(PREFIX)/bin
TARGET	:= txt2epub 
SOURCES := $(shell find src/ -type f -name *.c)
OBJECTS := $(patsubst src/%,build/%,$(SOURCES:.c=.o))
DEPS	:= $(OBJECTS:.o=.deps)
LDFLAGS := -Wl,--gc-sections
EXTRA_CFLAGS ?= 
EXTRA_LDFLAGS ?= 
CFLAGS  := -Wall -O3 -Wno-unused-result -ffunction-sections -fdata-sections -DVERSION=\"$(VERSION)\" -g -I include $(EXTRA_CFLAGS)

all: $(TARGET)

$(TARGET): $(OBJECTS) 
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJECTS) $(LIBS) $(EXTRA_LDFLAGS)

build/%.o: src/%.c
	@mkdir -p build/
	$(CC) $(CFLAGS) -MD -MF $(@:.o=.deps) -c -o $@ $<

clean:
	$(RM) -r build/ $(TARGET)

install: $(TARGET)
	mkdir -p ${BINDIR} ${MANDIR}/man1/
	install -s -D -m 755 $(TARGET) ${BINDIR}
	install -D -m 644 man1/* ${MANDIR}/man1/

-include $(DEPS)

.PHONY: clean

