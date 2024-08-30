/*
 * Your code here for context_switch
 *
 * It should restore the register values in the Trapframe with the 'iret' instruction.
 * This exits the kernel and starts executing some environment's code.
 *
 * This function does not return.
 */

.globl context_switch;
context_switch:
    // kernel to user
	mov 4(%esp), %esp // get the pointer to the Trapframe
	
    popal // restore the registers
    popl %es
	popl %ds

	addl $8, %esp // skip the error code and the pushed eip
	iret // return to user mode

spin:
	jmp spin
