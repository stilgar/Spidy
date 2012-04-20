CFLAGS=-Wall
LDFLAGS=-lm
ARCH=$(shell uname -m)

PROG=$(ARCH)/ragno6

all: arch $(PROG)

$(ARCH)/%: %.c
	gcc $(CFLAGS) -o $@ $^ $(LDFLAGS)

arch:
	mkdir -p $(ARCH)

clean:
	rm -Rf $(PROG) *.o $(ARCH)

.PHONY: clean
