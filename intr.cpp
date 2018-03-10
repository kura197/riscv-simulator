#include "intr.hpp"
#include "reg.h"
#include "emulator.hpp"
#include "csr.hpp"
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <iostream>

using namespace std;

void interrupt(Emulator* emu){

	//uart interrupt
	static int cnt;
	if(cnt++ > 100000){
		cnt = 0;

		char ch;		
		int n;
		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 1;
		FD_ZERO(&emu->kbd_fd);
		FD_SET(0, &emu->kbd_fd);

		if((select(1, &emu->kbd_fd, NULL, NULL, &tv)) > 0){
			if(FD_ISSET(0, &emu->kbd_fd)){
				if((n = read(0, &ch, sizeof(ch))) > 0){
					emu->store_mem8(IO_BASE+COM1, ch);
					emu->csr[mip] |= (1 << 11);
					//printf("kbd : %c\n",ch);
				}
			}
		}
	}

	//time interrupt
	if(emu->mtime >= emu->mtimecmp){
		emu->mtime = 0;
		emu->csr[mip] |= (1 << 7);
	}

	check_interrupt(emu, emu->runlevel, emu->csr[mideleg], emu->csr[mip], emu->csr[mie]);
}
void check_interrupt(Emulator* emu, int8_t runlevel, int32_t mideleg, int32_t mip, int32_t mie){
	int8_t old_runlevel = emu->runlevel;
	int ex_code = 0;
	bool interrupt = false;
	//interrupt
	if(MIE(emu->csr[mstatus]) == 1){
		switch(runlevel){
			case U:
				//software interrupt
				if(MSIP(mip) & MSIE(mie)){
					interrupt = true;
					ex_code = 0;
					emu->runlevel = M;
					//timer interrupt
				}else if(MTIP(mip) & MTIE(mie)){
					interrupt = true;
					ex_code = 4;
					emu->runlevel = M;
					//external interrupt
				}else if(MEIP(mip) & MEIE(mie)){
					interrupt = true;
					ex_code = 8;
					emu->runlevel = M;
				}
				break;
			case M:
					//software interrupt
				if(MSIP(mip) & MSIE(mie)){
					interrupt = true;
					ex_code = 3;
					//timer interrupt
				}else if(MTIP(mip) & MTIE(mie)){
					interrupt = true;
					ex_code = 7;
					//external interrupt
				}else if(MEIP(mip) & MEIE(mie)){
					interrupt = true;
					ex_code = 11;
				}
				break;
			default:
				cout << "error : priviledge level" << endl;
				break;
		}

		if(interrupt){
			//external interrupt
			if(ex_code == 11 || ex_code == 8){
				emu->csr[mcause] = 0x80000000 | ex_code;
				//emu->csr[mepc] = emu->V2P(emu->PC, -1);
				emu->csr[mepc] = emu->PC;
				emu->PC = (TVEC_MODE(emu->csr[mtvec]) == 0) ? emu->get_mem32(BASE(emu->csr[mtvec])) : emu->get_mem32(BASE(emu->csr[mtvec]) + 4*emu->read_exinterrupt());
				emu->csr[mstatus] |= MIE(emu->csr[mstatus]) << 7 ;
				emu->csr[mstatus] &= 0xfffffff7;
				emu->csr[mstatus] |= old_runlevel << 11 ;

			}else{
				emu->csr[mcause] = 0x80000000 | ex_code;
				emu->csr[mepc] = emu->V2P(emu->PC, -1);
				emu->PC = (TVEC_MODE(emu->csr[mtvec]) == 0) ? emu->get_mem32(BASE(emu->csr[mtvec])) : emu->get_mem32(BASE(emu->csr[mtvec]) + 4*ex_code);
				//MPIE(emu->csr[mstatus]) = MIE(emu->csr[mstatus]);
				//MIE(emu->csr[mstatus]) = 0;
				//MPP(emu->csr[mstatus]) = old_runlevel;
				emu->csr[mstatus] |= MIE(emu->csr[mstatus]) << 7 ;
				emu->csr[mstatus] &= 0xfffffff7;
				emu->csr[mstatus] |= old_runlevel << 11 ;
			}
#ifdef DEBUG
			cout << "interrupt occur!!  exception code = " << ex_code << endl;
#endif
		}
	}
	
}

/*
void check_interrupt(Emulator* emu, int8_t runlevel, int32_t mideleg, int32_t mip, int32_t mie){
	int ex_code = 0;
	int32_t old_mepc = emu->csr[mepc];
	int32_t old_sepc = emu->csr[sepc];
	//interrupt
	switch(runlevel){
		case U:
				//software interrupt
			if(MSIP(mip) & MSIE(mie)){
				ex_code = 0;
				if(USIP(mideleg)){
					emu->runlevel = S;
					emu->csr[scause] = 0x8000000 | ex_code;
				}else{
					emu->runlevel = M;
					emu->csr[mcause] = 0x8000000 | ex_code;

				}
				//timer interrupt
			}else if(MTIP(mip) & MTIE(mie)){
				ex_code = 4;
				if(UTIP(mideleg)){
					emu->runlevel = S;
					emu->csr[scause] = 0x8000000 | ex_code;

				}else{
					emu->runlevel = M;
					emu->csr[mcause] = 0x8000000 | ex_code;

				}
				//external interrupt
			}else if(MEIP(mip) & MEIE(mie)){
				ex_code = 8;
				if(UEIP(mideleg)){
					emu->runlevel = S;
					emu->csr[scause] = 0x8000000 | ex_code;

				}else{
					emu->runlevel = M;
					emu->csr[mcause] = 0x8000000 | ex_code;

				}
			}
			break;
		case S:
				//software interrupt
			if(MSIP(mip) & MSIE(mie)){
				ex_code = 1;
				if(SSIP(mideleg)){
					emu->csr[scause] = 0x8000000 | ex_code;

				}else{
					emu->runlevel = M;
					emu->csr[mcause] = 0x8000000 | ex_code;

				}
				//timer interrupt
			}else if(MTIP(mip) & MTIE(mie)){
				ex_code = 5;
				if(SSIP(mideleg)){
					emu->csr[scause] = 0x8000000 | ex_code;

				}else{
					emu->runlevel = M;
					emu->csr[mcause] = 0x8000000 | ex_code;

				}
				//external interrupt
			}else if(MEIP(mip) & MEIE(mie)){
				ex_code = 9;
				if(SSIP(mideleg)){
					emu->csr[scause] = 0x8000000 | ex_code;

				}else{
					emu->runlevel = M;
					emu->csr[mcause] = 0x8000000 | ex_code;

				}
			}
			break;
		case M:
			if(MSIP(mip) & MSIE(mie)){
				ex_code = 3;
				//software interrupt
			}else if(MTIP(mip) & MTIE(mie)){
				ex_code = 7;
				//timer interrupt
			}else if(MEIP(mip) & MEIE(mie)){
				ex_code = 11;
				//external interrupt
			}
			emu->csr[mcause] = 0x8000000 | ex_code;
			break;
		default:
			cout << "error : priviledge level" << endl;
			break;
	}

	emu->csr[mepc] = old_mepc;
	emu->csr[sepc] = old_sepc;
	
}
*/
