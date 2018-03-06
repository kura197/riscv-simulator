	.file	"test.c"
	.option nopic
	.text
	.align	2
	.globl	main
	.type	main, @function
main:
	addi	sp,sp,-144
	sw	s0,140(sp)
	addi	s0,sp,144
	li	a5,-2147479552
	addi	a5,a5,273
	sw	a5,-28(s0)
	li	a5,60
	sw	a5,-32(s0)
	lw	a5,-28(s0)
	addi	a5,a5,60
	sw	a5,-132(s0)
	lw	a5,-28(s0)
	xori	a5,a5,60
	sw	a5,-128(s0)
	lw	a5,-28(s0)
	andi	a5,a5,60
	sw	a5,-124(s0)
	lw	a5,-28(s0)
	ori	a5,a5,60
	sw	a5,-120(s0)
	lw	a5,-28(s0)
	slli	a5,a5,2
	sw	a5,-116(s0)
	lw	a5,-28(s0)
	srai	a5,a5,2
	sw	a5,-112(s0)
	lw	a5,-28(s0)
	srli	a5,a5,2
	sw	a5,-108(s0)
	lw	a4,-28(s0)
	lw	a5,-32(s0)
	add	a5,a4,a5
	sw	a5,-104(s0)
	lw	a4,-28(s0)
	lw	a5,-32(s0)
	sub	a5,a4,a5
	sw	a5,-100(s0)
	lw	a4,-28(s0)
	lw	a5,-32(s0)
	xor	a5,a4,a5
	sw	a5,-96(s0)
	lw	a4,-28(s0)
	lw	a5,-32(s0)
	and	a5,a4,a5
	sw	a5,-92(s0)
	lw	a4,-28(s0)
	lw	a5,-32(s0)
	or	a5,a4,a5
	sw	a5,-88(s0)
	lw	a4,-28(s0)
	lw	a5,-32(s0)
	sll	a5,a4,a5
	sw	a5,-84(s0)
	lw	a4,-28(s0)
	lw	a5,-32(s0)
	sra	a5,a4,a5
	sw	a5,-80(s0)
	lw	a5,-32(s0)
	srli	a5,a5,2
	sw	a5,-76(s0)
	lw	a4,-28(s0)
	lw	a5,-32(s0)
	sgt	a5,a4,a5
	andi	a5,a5,0xff
	sw	a5,-72(s0)
	lw	a4,-28(s0)
	lw	a5,-32(s0)
	sgtu	a5,a4,a5
	andi	a5,a5,0xff
	sw	a5,-68(s0)
	li	a5,1
	sw	a5,-36(s0)
	li	a5,-2147483648
	addi	a5,a5,2
	sw	a5,-40(s0)
	lw	a4,-36(s0)
	lw	a5,-40(s0)
	bne	a4,a5,.L2
	li	a5,1
	sw	a5,-64(s0)
	j	.L3
.L2:
	sw	zero,-64(s0)
.L3:
	lw	a4,-36(s0)
	lw	a5,-40(s0)
	beq	a4,a5,.L4
	li	a5,1
	sw	a5,-60(s0)
	j	.L5
.L4:
	sw	zero,-60(s0)
.L5:
	lw	a4,-36(s0)
	lw	a5,-40(s0)
	bge	a4,a5,.L6
	li	a5,1
	sw	a5,-56(s0)
	j	.L7
.L6:
	sw	zero,-56(s0)
.L7:
	lw	a4,-36(s0)
	lw	a5,-40(s0)
	blt	a4,a5,.L8
	li	a5,1
	sw	a5,-52(s0)
	j	.L9
.L8:
	sw	zero,-52(s0)
.L9:
	lw	a4,-36(s0)
	lw	a5,-40(s0)
	bleu	a4,a5,.L10
	li	a5,1
	sw	a5,-48(s0)
	j	.L11
.L10:
	sw	zero,-48(s0)
.L11:
	lw	a4,-36(s0)
	lw	a5,-40(s0)
	bleu	a4,a5,.L12
	li	a5,1
	sw	a5,-44(s0)
	j	.L13
.L12:
	sw	zero,-44(s0)
.L13:
	li	a5,4096
	addi	a5,a5,-256
	sw	a5,-20(s0)
	sw	zero,-24(s0)
	j	.L14
.L15:
	lw	a5,-24(s0)
	slli	a5,a5,2
	addi	a4,s0,-16
	add	a5,a4,a5
	lw	a4,-116(a5)
	lw	a5,-20(s0)
	sw	a4,0(a5)
	lw	a5,-20(s0)
	addi	a5,a5,4
	sw	a5,-20(s0)
	lw	a5,-24(s0)
	addi	a5,a5,1
	sw	a5,-24(s0)
.L14:
	lw	a4,-24(s0)
	li	a5,22
	ble	a4,a5,.L15
	li	a5,0
	mv	a0,a5
	lw	s0,140(sp)
	addi	sp,sp,144
.long 0x00000000
	jr	ra
	.size	main, .-main
	.ident	"GCC: (GNU) 7.2.0"
