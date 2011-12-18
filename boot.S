.section .boot, "ax"

.global _spidy_start
.extern __bss_start
.extern __bss_end
.extern spidy_setup
.extern spidy_main
.extern spidy_step

_spidy_start:
/* set the stack pointer */
	mov r0, pc
	sub r0, #12
	mov sp, r0

/* clear BSS */
	mov r0, #0
	ldr r1, =__bss_start
	ldr r2, =__bss_end
0:
	cmp r1, r2
	bge 1f
	str r0, [r1]
	add r1, #4
	b 0b

/* execute setup, main and looping step */
1:	bl spidy_setup
	bl spidy_main
2:	bl spidy_step
	b 2b
