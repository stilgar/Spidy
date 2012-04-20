/*
 * This program includes the initial assembly stuff. It sets the stack
 * pointer and zeroes the BSS. For rom-boot it also copies data to RAM.
 *
 * Different ELF sections are used to differentiate the behaviour
 */

.extern spidy_setup
.extern spidy_main
.extern spidy_step
// .extern spidy_reset

/* RAM boot: the first kB of RAM is unused, place the stack here */
.section .text.ramboot
.global _entry_ram
_entry_ram:
	mov	r0, pc
	sub	r0, #12
	mov	sp, r0
	b	_bss_loop

/* ROM boot: put the stack before data (the lds puts data 1k into RAM) */
.section .text.romboot
.global _entry_rom
_entry_rom:
	/* FIXME: check flash speed */
	ldr	r0, =_sdata
	ldr	r1, =_edata
	ldr	r2, =_erom
	/* stack is before data */
	mov	r3, r0
	sub	r3, #4
	mov	sp, r3
	/* copy from r2 to r0, until r1. We know it's aligned at 16b */
10:
	cmp	r0, r1
	bge	_bss_loop
	ldmia	r2!, {r4, r5, r6, r7}
	stmia	r0!, {r4, r5, r6, r7}
	b	10b
.ltorg

/* What follows is common to ROM and RAM boot: clear bss and call main */
.section .text.anyboot
_bss_loop:
	ldr	r1, =__bss_start
	ldr	r2, =__bss_end
	mov	r0, #0
0:
	cmp	r1, r2
	bge	1f
	str	r0, [r1]
	add	r1, #4
	b	0b
1:
	//bl spidy_reset
	/* execute setup, main and looping step */
	mov	r0, #0
	bl spidy_setup
	bl spidy_main
2:
	bl spidy_step
	b 2b
