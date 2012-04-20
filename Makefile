# Using thumb for version 7 ARM core:
CFLAGS  = -march=armv7-m -mthumb -Wall -ffreestanding -ggdb -O2
ASFLAGS = -march=armv7-m -mthumb -Wall -ggdb
OMPFLAGS = -fopenmp
# Use our own linker script
RAMLDFLAGS = -Bstatic -T spidy-ram.lds
ROMLDFLAGS = -Bstatic -T spidy-rom.lds

# Host compiling:
HCC = gcc
HCFLAGS = -ggdb -Wall -O2
HLDFLAGS = -lrt -lm

# Clean particular files
CLEANSTUFF = pipe

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

BASE = micro

OBJ = boot.o io.o utils.o gpio.o vectors.o
OBJ += micro.o

EXE = $(BASE).ram $(BASE).rom
BIN = $(BASE).ram.bin $(BASE).rom.bin

HCPROG = sender master

all: $(BIN) $(HCPROG) $(patsubst test-%.c,test-%,$(wildcard test-*)) tools

######### HOST COMPILING
pc.o: pc.c
	$(HCC) $(HCFLAGS) -c $^

walk.o: walk.c
	$(HCC) $(HCFLAGS) -c $^

calibrate.o: calibrate.c
	$(HCC) $(HCFLAGS) -c $^

sender.o: sender.c
	$(HCC) $(HCFLAGS) -c $^

master.o: master.c
	$(HCC) $(HCFLAGS) -c $^

sender: sender.o pc.o
	$(HCC) $^ $(HLDFLAGS) -o $@

master: master.o pc.o walk.o calibrate.o
	$(HCC) $^ $(HLDFLAGS) -o $@

######### CROSS-COMPILING
$(BASE).ram: $(OBJ)
	$(LD) $(RAMLDFLAGS) $^ -o $@

$(BASE).rom: $(OBJ)
	$(LD) $(ROMLDFLAGS) $^ -o $@

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
	rm -f $(BIN) $(EXE) $(HCPROG) $(CLEANSTUFF) *.o *~
	$(MAKE) -C tools clean

.PHONY: tools clean
