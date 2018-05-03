#include "intr.hpp"
#include "reg.h"
#include "emulator.hpp"
#include "csr.hpp"
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <iostream>
#include <queue>

using namespace std;
//#define DEBUG

void interrupt(Emulator* emu){

	//uart interrupt
	static int cnt;
	if(cnt++ > 100000){
		cnt = 0;

		int8_t ch;		
		int n;
		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 1;
		FD_ZERO(&emu->kbd_fd);
		FD_SET(0, &emu->kbd_fd);

		if((select(1, &emu->kbd_fd, NULL, NULL, &tv)) > 0){
			if(FD_ISSET(0, &emu->kbd_fd)){
				if((n = read(0, &ch, sizeof(ch))) > 0){
				//if((ch = getchar()) >= 0){
					//emu->store_mem8(IO_BASE+COM1, ch);
					emu->csr[mip] |= (1 << 11);
					emu->uart_rx.push(ch);
					//printf("kbd : %c, buf:%d\n",ch, emu->uart_rx.empty());
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
				emu->csr[mcause] = 0x80000000 | ex_code;
				emu->csr[mepc] = emu->PC;
				emu->PC = (TVEC_MODE(emu->csr[mtvec]) == 0) ? emu->get_mem32(BASE(emu->csr[mtvec])) : emu->get_mem32(BASE(emu->csr[mtvec]) + 4*ex_code);
				//emu->PC = emu->get_mem32(BASE(emu->csr[mtvec]));
				emu->csr[mstatus] |= MIE(emu->csr[mstatus]) << 7 ;
				emu->csr[mstatus] &= 0xfffffff7;
				emu->csr[mstatus] |= old_runlevel << 11 ;
#ifdef DEBUG
			cout << "interrupt occur!!  exception code = " << ex_code << endl;
			cout << "MSIP = " << MSIP(mip) << endl;
			cout << "MTIP = " << MTIP(mip) << endl;
			cout << "MEIP = " << MEIP(mip) << endl;
#endif
		}
	}
	
}
