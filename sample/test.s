	.file	"test.c"
	.option nopic
	.text
	.align	2
	.globl	main
	.type	main, @function
main:
	addi	sp,sp,-32
	sw	s0,28(sp)
	addi	s0,sp,32
	li	a5,15
	sw	a5,-20(s0)
	lw	a4,-20(s0)
	li	a5,14
	beq	a4,a5,.L2
	lw	a4,-20(s0)
	li	a5,13
	beq	a4,a5,.L2
	lw	a4,-20(s0)
	li	a5,12
	beq	a4,a5,.L2
	lw	a5,-20(s0)
	addi	a5,a5,1
	sw	a5,-20(s0)
.L2:
	li	a5,-1
	mv	a0,a5
	lw	s0,28(sp)
	addi	sp,sp,32
	jr	ra
	.size	main, .-main
	.ident	"GCC: (GNU) 7.2.0"
