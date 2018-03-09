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
	li	a5,1953067008
	addi	a5,a5,-407
	sw	a5,-32(s0)
	sb	zero,-28(s0)
	addi	a5,s0,-32
	addi	a5,a5,100
	sw	a5,-24(s0)
	addi	a5,s0,-32
	sw	a5,-20(s0)
	j	.L2
.L5:
	lw	a5,-20(s0)
	lbu	a5,0(a5)
	bnez	a5,.L3
	lw	a4,-20(s0)
	addi	a5,s0,-32
	sub	a5,a4,a5
	j	.L6
.L3:
	lw	a5,-20(s0)
	addi	a5,a5,1
	sw	a5,-20(s0)
.L2:
	lw	a4,-20(s0)
	lw	a5,-24(s0)
	bltu	a4,a5,.L5
	li	a5,-1
.L6:
	mv	a0,a5
	lw	s0,28(sp)
	addi	sp,sp,32
	jr	ra
	.size	main, .-main
	.ident	"GCC: (GNU) 7.2.0"
