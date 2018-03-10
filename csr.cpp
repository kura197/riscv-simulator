#include "csr.hpp"
#include <iostream>

using namespace std;

int32_t num2csr(int32_t num, int8_t runlevel){
	switch(num){
//machine//
		//mimpid(external interrupt)
		case 0xf13:
			if(runlevel > 2)
				return mstatus;
			else
				return 0;
		//mstatus  
		case 0x300:
			if(runlevel > 2)
				return mstatus;
			else
				return 0;
		//medeleg  
		case 0x302:
			if(runlevel > 2)
				return medeleg;
			else
				return 0;
		//mideleg  
		case 0x303:
			if(runlevel > 2)
				return mideleg;
			else
				return 0;
		//mie  
		case 0x304:
			if(runlevel > 2)
				return mie;
			else
				return 0;
		//mtvec  
		case 0x305:
			if(runlevel > 2)
				return mtvec;
			else
				return 0;
		//mscratch  
		case 0x340:
			if(runlevel > 2)
				return mscratch;
			else
				return 0;
		//mepc  
		case 0x341:
			if(runlevel > 2)
				return mepc;
			else
				return 0;
		//mcause  
		case 0x342:
			if(runlevel > 2)
				return mcause;
			else
				return 0;
		////mtval  
		//case 0x343:
		//	if(runlevel > 2)
		//		return mtval;
		//	else
		//		return 0;
		//mip  
		case 0x344:
			if(runlevel > 2)
				return mip;
			else
				return 0;

//SuperViser//
		//sstatus  
		case 0x100:
			if(runlevel > 0)
				return sstatus;
			else
				return 0;
		//sedeleg  
		case 0x102:
			if(runlevel > 0)
				return sedeleg;
			else
				return 0;
		//sideleg  
		case 0x103:
			if(runlevel > 0)
				return sideleg;
			else
				return 0;
		//sie  
		case 0x104:
			if(runlevel > 0)
				return sie;
			else
				return 0;
		//stvec  
		case 0x105:
			if(runlevel > 0)
				return stvec;
			else
				return 0;
		//sscratch  
		case 0x140:
			if(runlevel > 0)
				return sscratch;
			else
				return 0;
		//sepc  
		case 0x141:
			if(runlevel > 0)
				return sepc;
			else
				return 0;
		//scause  
		case 0x142:
			if(runlevel > 0)
				return scause;
			else
				return 0;
		//satp  
		case 0x180:
			if(runlevel > 0)
				return satp;
			else
				return 0;


		default:
			cout << "this register is not implemented" << endl;
			return 0;
	}
}
