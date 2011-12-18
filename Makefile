# Using thumb for version 7 ARM core:
CFLAGS  = -march=armv7-m -mthumb -g -Wall -ffreestanding -ggdb -O2
ASFLAGS = -march=armv7-m -mthumb -g -Wall
# Use our own linker script
LDFLAGS = -Bstatic -T spidy.lds

# Host compiling:
HCC = gcc
HCFLAGS = -ggdb -Wall -O2
HLDFLAGS =

# Cross compiling:
AS		= $(CROSS_COMPILE)as
LD		= $(CROSS_COMPILE)ld
CC		= $(CROSS_COMPILE)gcc
CPP		= $(CC) -E
AR		= $(CROSS_COMPILE)ar
NM		= $(CROSS_COMPILE)nm
STRIP		= $(CROSS_COMPILE)strip
OBJCOPY		= $(CROSS_COMPILE)objcopy
OBJDUMP		= $(CROSS_COMPILE)objdump

BASE = spidy

OBJ = boot.o io.o utils.o gpio.o
OBJ += spidy.o

EXE = $(BASE).ram
BIN = $(BASE).ram.bin

PROG = sender


all: $(BIN) $(patsubst sender%.c,sender%,$(wildcard sender*)) $(patsubst test-%.c,test-%,$(wildcard test-*)) tools 

######### HOST COMPILING
sender: sender.c
	$(HCC) $(HCFLAGS) $^ $(HLDFLAGS) -o $@

sender-thread: sender-thread.c
	$(HCC) $(HCFLAGS) -fopenmp $^ $(HLDFLAGS) -o $@

######### CROSS-COMPILING
$(BASE).ram: $(OBJ)
	$(LD) $(LDFLAGS) $^ -o $@

%.bin: % tools
	$(OBJCOPY) -O binary $* $@
	@if echo $@ | grep -q rom.bin; then ./tools/fix-checksum $@; fi

tools:
	$(MAKE) -C tools

test-%: $(BIN)
	$(MAKE) -C $@ all

.PHONY: all

$(BIN): $(EXE)

clean:
	rm -f $(BIN) $(EXE) $(PROG) *.o *~ sender
	$(MAKE) -C tools clean

.PHONY: tools clean
