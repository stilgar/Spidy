.global vectors

.section .vectors, "ax"
.extern _entry_rom

vectors:
	/* The first vector in Cortex is the stack pointer */
	.word 0x100003fc
	/* The second one is reset */
	.word _entry_rom+1

	/*
	 * We don't have interrupts, so there's no need to allocate
	 * vectors. However, we neet at least 8 of them, as vector 7
	 * is used for the checksum. So align to 0x20
	 */
	.word 0xcafefeed
.align 5, 0
