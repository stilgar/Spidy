# Using thumb for version 7 ARM core:
CFLAGS  = -march=armv7-m -mthumb -g -Wall -ffreestanding -ggdb -O2
ASFLAGS = -march=armv7-m -mthumb -g -Wall
# Use our own linker script
LDFLAGS = -Bstatic -T ../spidy.lds

# Cross compiling:
AS              = $(CROSS_COMPILE)as
LD              = $(CROSS_COMPILE)ld
CC              = $(CROSS_COMPILE)gcc
CPP             = $(CC) -E
AR              = $(CROSS_COMPILE)ar
NM              = $(CROSS_COMPILE)nm
STRIP           = $(CROSS_COMPILE)strip
OBJCOPY         = $(CROSS_COMPILE)objcopy
OBJDUMP         = $(CROSS_COMPILE)objdump

BASE = spidy-test

OBJ = ../boot.o ../io.o ../utils.o ../gpio.o
OBJ += spidy-test.o

EXE = $(BASE).ram
BIN = $(BASE).ram.bin


# Host compiling:
HCC ?= gcc
HCFLAGS = -ggdb -Wall -O2
HLDFLAGS = 

PROG = test-sender



all: $(BIN) $(PROG)

######### HOST COMPILING
$(PROG): $(patsubst %,%.c,$(PROG))
	$(HCC) $(HCFLAGS) $^ $(HLDFLAGS) -o $@

######### CROSS-COMPILING
$(BASE).ram: $(OBJ)
	$(LD) $(LDFLAGS) $^ -o $@

%.bin: % tools
	$(OBJCOPY) -O binary $* $@
	@if echo $@ | grep -q rom.bin; then ./tools/fix-checksum $@; fi

.PHONY: all

$(BIN): $(EXE)

clean:
	rm -f $(BIN) $(EXE) *.o *~ $(PROG)

.PHONY: clean
