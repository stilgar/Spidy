OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_entry_rom)

regs = 0;
jiffies = 0x40018008;

SECTIONS
{
	. = 0x0;
	.vectors : { *(.vectors); }
	.text.boot : {
		*(.text.romboot)
		*(.text.anyboot)
	}
	.text : { *(.text) }
	.rodata : { *(.rodata) }
	. = ALIGN(16);
	_erom = .;

	.data 0x10000400: AT(_erom) {
		_sdata = .;
		*(.data);
		_edata = .;
	}

	.bss : {
		. = ALIGN(16);
		__bss_start = .;
		*(.bss);
		. = ALIGN(16);
		__bss_end = .;
	}

	/DISCARD/ : { *(.text.ramboot); }
}
