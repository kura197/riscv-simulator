	.file	"test.c"
	.option nopic
	.text
	.align	2
	.globl	main
	.type	main, @function
main:
	addi	sp,sp,-112
	sw	ra,108(sp)
	sw	s0,104(sp)
	addi	s0,sp,112
	li	a5,16
	sw	a5,-60(s0)
	li	a5,48
	sw	a5,-24(s0)
	lw	t3,-60(s0)
	lw	t1,-56(s0)
	lw	a7,-52(s0)
	lw	a6,-48(s0)
	lw	a0,-44(s0)
	lw	a1,-40(s0)
	lw	a2,-36(s0)
	lw	a3,-32(s0)
	lw	a4,-28(s0)
	lw	a5,-24(s0)
	sw	t3,-112(s0)
	sw	t1,-108(s0)
	sw	a7,-104(s0)
	sw	a6,-100(s0)
	sw	a0,-96(s0)
	sw	a1,-92(s0)
	sw	a2,-88(s0)
	sw	a3,-84(s0)
	sw	a4,-80(s0)
	sw	a5,-76(s0)
	addi	a5,s0,-112
	mv	a0,a5
	call	sum
	sw	a0,-20(s0)
	li	a5,0
	mv	a0,a5
	lw	ra,108(sp)
	lw	s0,104(sp)
	addi	sp,sp,112
	jr	ra
	.size	main, .-main
	.align	2
	.globl	sum
	.type	sum, @function
sum:
	addi	sp,sp,-32
	sw	s0,28(sp)
	sw	s1,24(sp)
	addi	s0,sp,32
	mv	s1,a0
	lw	a5,36(s1)
	sw	a5,-20(s0)
	lw	a5,-20(s0)
	mv	a0,a5
	lw	s0,28(sp)
	lw	s1,24(sp)
	addi	sp,sp,32
	jr	ra
	.size	sum, .-sum
	.ident	"GCC: (GNU) 7.2.0"
