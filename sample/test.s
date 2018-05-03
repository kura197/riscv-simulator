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
# set MXR 1
	li t1,0x80000
	csrs mstatus,t1
	la	  t0,entrypgdir
	#srli    t0,t0,12
	csrw    satp, t0
# Turn on paging.
	csrr    t0,satp
	li	 t1,0x80000000
	or     t0,t0,t1
	csrw    satp, t0

	li	a5,1
	csrr a4,mstatus
	or a4,a4,a5
	csrw mstatus,a4
	li	a5,8
	csrs mstatus,a5
	li	a5,1
	csrs mstatus,a5
	csrw mstatus,a5
	nop
	lw	s0,28(sp)
	addi	sp,sp,32
.long 0x00000000
	jr	ra
	.size	main, .-main
	.ident	"GCC: (GNU) 7.2.0"

entrypgdir:
	.long 0x0000000f
