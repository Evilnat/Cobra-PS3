	.text
	.section .start,"ax",@progbits
	.align 3

	.global _start
	
_start:
	mflr	%r0
	std 	%r0, 0x10(%sp)
	stdu 	%sp, -0x80(%sp)
	std 	%rtoc, 0x28(%sp)
	clrrdi 	%rtoc, %rtoc, 32
	oris 	%rtoc, %rtoc, __toc@h
	ori 	%rtoc, %rtoc, __toc@l
	bl 	main
	ld	%rtoc, 0x28(%sp)
	addi 	%sp, %sp, 0x80
	ld 	%r0, 0x10(%sp)	
	mtlr 	%r0
	blr
	
	
	

