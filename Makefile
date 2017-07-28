OBJECTS = $(SOURCES:.c=.o)
TARGETS = $(OBJECTS:.o=)

.PHONY: all clean

include sources.mk

all: $(TARGETS)

clean:
	-rm $(TARGETS) $(OBJECTS)

COMBINED_CFLAGS= $(CPPFLAGS) $(CFLAGS)
AUG_CFLAGS = $(COMBINED_CFLAGS)

.c.o:
	$(CC) $(AUG_CFLAGS) -c $<

.SUFFIXES: .html .md
.md.html:
	markdown < $< > $@

LIB =
include dependencies.mk
include targets.mk

include maintainer.mk # Rules not required for just building the application.
