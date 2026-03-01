#include "mm.h"

.section ".text.boot" // specify that everything in boot.S should go in .text.boot section. this section is put at the start of the kernel image by the linker script

.globl _start
_start:
// mrs loads a value from a system register to one of the general purpose registers (x0 - x30)
// and performs a logical and. we use this to get the last byte from the value received from mpidr_el1
// cbz compares the result of the previously executed operator to 0 and branches to the provided label if true
// b performs an uncondition branch to a provided label
// adr loads a label's relative address into a target register. here, we want the pointer to start at the end of .bss
// sub subtracts
// bl "branch with link" - performs an unconditional branch and stores the return address in x30 (the link register). when the subroutine is finished, it uses the ret instr to jump to the return address
// mov movs
	mrs	x0, mpidr_el1 // gets processor id from mpidr_el1 system register
	and	x0, x0,#0xFF // check processor id
	cbz	x0, master // hang for all non-primary CPU
	b	proc_hang 

prog_hang:
	b proc_hang


// this function cleans the .bss section by calling memzero
// memzero accepts two args: the start address (bss_begin)
// and the size of the area that needs to be clearned (bss_end - bss_begin)
master:
	adr	x0, bss_begin
	adr	x1, bss_end
	sub	x1, x1, x0
	bl	memzero

// after cleaning the .bss section, we intialize the stack pointer
// and execute the kernel_main function
	mov	sp, #LOW_MEMORY // LOW_MEMORY is defined in mm.h and is = to 4mb
	bl	kernel_main

