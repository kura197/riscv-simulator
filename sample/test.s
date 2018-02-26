	.file	"test.c"
	.option nopic
	.text
	.align	2
	.globl	main
	.type	main, @function
main:
	addi	sp,sp,-48
	sw	s0,44(sp)
	addi	s0,sp,48
	li	a5,5
	sw	a5,-20(s0)
	sw	zero,-24(s0)
	li	a5,-2147483648
	sw	a5,-28(s0)
	li	a5,240
	sw	a5,-32(s0)
	li	a5,4096
	sw	a5,-36(s0)
	lw	a5,-36(s0)
	lw	a4,-20(s0)
	sw	a4,0(a5)

	lw  a5,-28(s0)
	csrw satp,a5 
	lw	a5,-24(s0)
	lw	a4,-20(s0)
	sw	a4,0(a5)
	lw  a5,-28(s0)
	csrc satp,a5 

	lw	a5,-32(s0)
	lw	a4,-24(s0)
	sw	a4,0(a5)
	li	a5,0
	mv	a0,a5
	lw	s0,44(sp)
	addi	sp,sp,48
.long 0x00000000
	mret
	jr	ra
	.size	main, .-main
	.ident	"GCC: (GNU) 7.2.0"
