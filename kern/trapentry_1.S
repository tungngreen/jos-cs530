/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>
#include <kern/macro.h>



###################################################################
# exceptions/interrupts
###################################################################

/* TRAPHANDLER defines a globally-visible function for handling a trap.
 * It pushes a trap number onto the stack, then jumps to _alltraps.
 * Use TRAPHANDLER for traps where the CPU automatically pushes an error code.
 *
 * You shouldn't call a TRAPHANDLER function from C, but you may
 * need to _declare_ one in C (for instance, to get a function pointer
 * during IDT setup).  You can declare the function with
 *   void NAME();
 * where NAME is the argument passed to TRAPHANDLER.
 */
#define TRAPHANDLER(name, num)						\
	.globl name;		/* define global symbol for 'name' */	\
	.type name, @function;	/* symbol type is function */		\
	.align 2;		/* align function definition */		\
	name:			/* function starts here */		\
	pushq $(num);							\
	jmp _alltraps

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */
#define TRAPHANDLER_NOEC(name, num)					\
	.globl name;							\
	.type name, @function;						\
	.align 2;							\
	name:								\
	pushq $0;							\
	pushq $(num);							\
	jmp _alltraps

.text

/*
 * Lab 3: Your code here for generating entry points for the different traps.
   Find the PUSHA,POPA macros in kern/macro.h. Since amd64 doesn't support
   pusha,popa so you will find these macros handy.
 */

	TRAPHANDLER_NOEC(T16_DIVIDE,0);
	TRAPHANDLER_NOEC(T16_DEBUG,1)
	TRAPHANDLER_NOEC(T16_NMI,2)
	TRAPHANDLER_NOEC(T16_BRKPT,3)
	TRAPHANDLER_NOEC(T16_OFLOW,4)
	TRAPHANDLER_NOEC(T16_BOUND,5)
	TRAPHANDLER_NOEC(T16_ILLOP,6)
	TRAPHANDLER_NOEC(T16_DEVICE,7)
	TRAPHANDLER(T16_DBLFLT,8)
	TRAPHANDLER(T16_TSS,10)
	TRAPHANDLER(T16_SEGNP,11)
	TRAPHANDLER(T16_STACK,12)
	TRAPHANDLER(T16_GPFLT,13)
	TRAPHANDLER(T16_PGFLT,14)
	TRAPHANDLER_NOEC(T16_FPERR,16)
	TRAPHANDLER_NOEC(T16_ALIGN,17)
	TRAPHANDLER_NOEC(T16_MCHK,18)
	TRAPHANDLER_NOEC(T16_SIMDERR,19)
	TRAPHANDLER_NOEC(T16_SYSCALL,48)



/*
 * Lab 3: Your code here for _alltraps
 *
 * Hint: Be sure to review the x64 calling convention from lab1
 *       for how to pass a pointer to the trapframe.
 */
	
_alltraps:
	subq $8,%rsp
	movq $0, (%rsp)
	mov %ds,(%rsp)		
	subq $8,%rsp
	movq $0, (%rsp)
	mov %es,(%rsp)
	PUSHA
	mov %rsp,%rdi
	call trap
	POPA_
	mov (%rsp),%es
	addq $8, %rsp
	mov (%rsp), %ds
	addq $8, %rsp	

