#ifndef REG_H_
#define REG_H_

//PTE
#define D(x) ((x >> 7) & 1)
#define A(x) ((x >> 6) & 1) 
#define G(x) ((x >> 5) & 1)
#define U(x) ((x >> 4) & 1)
#define X(x) ((x >> 3) & 1)
#define W(x) ((x >> 2) & 1)
#define R(x) ((x >> 1) & 1)
#define V(x) ((x >> 0) & 1)

//satp
#define PPN(x) (x & 0x3fffff)
#define ASID(x) ((x >> 22) & 0x1ff)
#define MODE(x) (x >> 31)

//xstatus
#define UIE(x) ((x >> 0) & 1)
#define SIE(x) ((x >> 1) & 1) 
#define MIE(x) ((x >> 3) & 1)
#define UPIE(x) ((x >> 4) & 1)
#define SPIE(x) ((x >> 5) & 1)
#define MPIE(x) ((x >> 7) & 1)
#define SPP(x) ((x >> 8) & 1)
#define MPP(x) ((x >> 11) & 0b11)
#define MPRV(x) ((x >> 17) & 1)
#define SUM(x) ((x >> 18) & 1) 
#define MXR(x) ((x >> 19) & 1)
#define TVM(x) ((x >> 20) & 1)
#define TW(x) ((x >> 21) & 1)
#define TSR(x) ((x >> 22) & 1)

//xip 
#define USIP(x) ((x >> 0) & 1)
#define SSIP(x) ((x >> 1) & 1) 
#define MSIP(x) ((x >> 3) & 1)
#define UTIP(x) ((x >> 4) & 1)
#define STIP(x) ((x >> 5) & 1)
#define MTIP(x) ((x >> 7) & 1)
#define UEIP(x) ((x >> 8) & 1)
#define SEIP(x) ((x >> 9) & 1)
#define MEIP(x) ((x >> 11) & 1)

//xie 
#define USIE(x) ((x >> 0) & 1)
#define SSIE(x) ((x >> 1) & 1) 
#define MSIE(x) ((x >> 3) & 1)
#define UTIE(x) ((x >> 4) & 1)
#define STIE(x) ((x >> 5) & 1)
#define MTIE(x) ((x >> 7) & 1)
#define UEIE(x) ((x >> 8) & 1)
#define SEIE(x) ((x >> 9) & 1)
#define MEIE(x) ((x >> 11) & 1)

//xtvec
#define BASE(x) (x >> 2)
#define TVEC_MODE(x) (x & 0b11)





#endif
