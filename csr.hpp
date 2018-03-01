#ifndef _CSR_H_
#define _CSR_H_

#include <stdint.h>

#define CSR_CNT 20 + 1

//mimpidをexternal interrupt用のレジスタとして代用
enum CSRs{mstatus = 1, medeleg, mideleg, mie, mtvec, mscratch, mepc, mcause, mip, sstatus, sedeleg, sideleg, sie, stvec, sscratch, sepc, scause, sip, satp, mimpid};
int32_t num2csr(int32_t num, int8_t runlevel);

#endif
