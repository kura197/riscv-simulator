#include<stdint.h>
#include<string>
#include<vector>
#include<stdio.h>
#include<iostream>
#include<fstream>
#include "csr.hpp"
#include "reg.h"
#include "emulator.hpp"

using namespace std;


Emulator::Emulator(){
	clear_registers();
	//always 0
	x[0] = 0;
	//stack pointer
	x[2] = FIRST_SP;
	PC = STARTPC;
	runlevel = M;
	csr[misa] = 0x40001101;
	memory = new uint8_t[MEMSIZE];
	io_mem = new uint8_t[0 - IO_BASE];
}

Emulator::~Emulator(){
	delete memory;
}

void Emulator::load_memory(ifstream *binary, uint32_t bin_addr, uint32_t mem_addr, int size){
	int memsize = 0;
	uint8_t* memory_ptr = memory;
	memory_ptr += mem_addr;
	binary->seekg(bin_addr * sizeof(int8_t));
	while(!binary->eof() && memsize < size){
		binary->read((char*)memory_ptr, sizeof(uint8_t));
		memory_ptr++;
		memsize++;
	}
	binary->seekg(0,ios_base::beg);
}

void Emulator::clear_registers(){
	for(int i=0;i<USER_REG_CNT;i++)
		x[i] = 0;
	for(int i=0;i<CSR_CNT;i++)
		csr[i] = 0;
}

uint32_t Emulator::fetch(){
	uint32_t instr;
	instr = (memory[V2P(PC+3, PAGE_X)] << 24)|(memory[V2P(PC+2, PAGE_X)] << 16)|(memory[V2P(PC+1, PAGE_X)] << 8)|(memory[V2P(PC, PAGE_X)]);
	x[0] = 0;
	PC += 4;

	mtime += CYCLE;
	return instr;
}

void Emulator::dump_registers(int num){
	switch(num){
		case 0:
			printf(" ra = %08x, sp = %08x, s0 = %08x, s1 = %08x	\n",x[1],x[2],x[8],x[9]);
			printf(" a0 = %08x, a1 = %08x, a2 = %08x, a3 = %08x	\n",x[10],x[11],x[12],x[13]);
			printf(" a4 = %08x, a5 = %08x, a6 = %08x, a7 = %08x	\n",x[14],x[15],x[16],x[17]);
			break;
		default:
			for(int i=0;i<USER_REG_CNT;i++){
				if(i % DUMP_ROW == 0)	printf("\n");
				if(i < 10)
					printf(" x%d=%08x	",i,x[i]);
				else
					printf("x%d=%08x	",i,x[i]);
			}
			printf("\n");
			break;
	}
}

void Emulator::dump_memory(int32_t start_addr, size_t limit){
	int i, k;
	if(start_addr >= IO_BASE){
		start_addr = start_addr - IO_BASE;
		for(i = start_addr,k = 0;i < start_addr + limit; i++, k++){
			if(k % DUMP_ROW == 0)	printf("\n");
			printf("%08x:%02x	",i,memory[i]);
		}
		printf("\n");
	}else{
		for(i = start_addr,k = 0;i < start_addr + limit; i++, k++){
			if(k % DUMP_ROW == 0)	printf("\n");
			printf("%08x:%02x	",i,memory[i]);
		}
		printf("\n");
	}
}

int32_t Emulator::get_mem32(uint32_t addr){
	if(addr >= IO_BASE){
		addr = addr - IO_BASE;
		return (io_mem[addr+3] << 24)|(io_mem[addr+2] << 16)|(io_mem[addr+1] << 8)|(io_mem[addr]);
	}else
		return (memory[V2P(addr+3, PAGE_R)] << 24)|(memory[V2P(addr+2, PAGE_R)] << 16)|(memory[V2P(addr+1, PAGE_R)] << 8)|(memory[V2P(addr, PAGE_R)]);
}

int16_t Emulator::get_mem16(uint32_t addr){
	if(addr >= IO_BASE){
		addr = addr - IO_BASE;
		return (io_mem[addr+1] << 8)|(io_mem[addr]);
	}else
		return (memory[V2P(addr+1, PAGE_R)] << 8)|(memory[V2P(addr, PAGE_R)]);
}

int8_t Emulator::get_mem8(uint32_t addr){
	if(addr >= IO_BASE){
		addr = addr - IO_BASE;
		//UART
		if(addr == COM1){
			int8_t uart = uart_rx.front();
			uart_rx.pop();
			return uart;
		}else if(addr == COM1+5){
			int8_t LSR = (uart_rx.empty()) ? 0x0 : 0x01;
			return LSR;
		}else
			return io_mem[addr];
	}else
		return memory[V2P(addr, PAGE_R)];
}

void Emulator::store_mem32(uint32_t addr, int32_t value){
	if(addr >= IO_BASE){
		addr = addr - IO_BASE;
		switch(addr){
			case MTIME_L:
				mtime &= ~0xffffffff;
				mtime |= value;
				break;
			case MTIME_H:
				mtime &= 0xffffffff;
				mtime |= (uint64_t)value << 32;
				break;
			case MTIMECMP_L:
				mtimecmp &= ~0xffffffff;
				mtimecmp |= value;
				break;
			case MTIMECMP_H:
				mtimecmp &= 0xffffffff;
				mtimecmp |= (uint64_t)value << 32;
				break;
		}
		for(int i = 0; i < 4; i++){
			io_mem[addr] = ((value >> 8*i) & 0xff);
			addr++;
		}
	}else
		for(int i = 0; i < 4; i++){
			memory[V2P(addr, PAGE_W)] = (value >> 8*i & 0xff);
			addr++;
		}
}

void Emulator::store_mem16(uint32_t addr, int16_t value){
	if(addr >= IO_BASE){
		addr = addr - IO_BASE;
		for(int i = 0; i < 2; i++){
			io_mem[addr] = ((value >> 8*i) & 0xff);
			addr++;
		}
	}else
		for(int i = 0; i < 2; i++){
			memory[V2P(addr, PAGE_W)] = ((value >> 8*i) & 0xff);
			addr++;
		}
}

void Emulator::store_mem8(uint32_t addr, int8_t value){
	if(addr >= IO_BASE){
		addr = addr - IO_BASE;
		//UART
		if(addr == COM1){
			uart_tx.push(value);
			//printf("%c",value);
			//fflush(stdout);
		}else
			io_mem[addr] = value;
	}else
		memory[V2P(addr, PAGE_W)] = value;
}

//mimpic(external interrupt)
uint8_t Emulator::read_exinterrupt(){
	uint8_t ex = (csr[mimpid] & 0xff);
	return ex;
}

void Emulator::set_exinterrupt(int8_t num){
	csr[mimpid] = num;
}

int32_t Emulator::get_phys_mem32(int32_t addr){
	if(addr >= IO_BASE){
		addr = addr - IO_BASE;
		return (io_mem[addr+3] << 24)|(io_mem[addr+2] << 16)|(io_mem[addr+1] << 8)|io_mem[addr];
	}else
		return (memory[addr+3] << 24)|(memory[addr+2] << 16)|(memory[addr+1] << 8)|memory[addr];
}

uint32_t Emulator::V2P(uint32_t va, int mode){
	if(MODE(csr[satp]) == 0)
		return va;
	uint32_t pa;
	uint32_t vpn[2];
	uint16_t ppn[2];
	uint16_t va_offset;
	uint16_t pa_offset;
	vpn[1] = (va & 0xffc00000) >> 22;
	vpn[0] = (va & 0x003ff000) >> 12;
	pa_offset = va & 0x00000fff;

	uint32_t a = (int32_t)PPN(csr[satp]) * PAGESIZE;
	uint32_t i = LEVELS - 1;
	uint32_t pte;
	uint32_t old_pte;
	while(1){
		pte = (uint32_t)(a + vpn[i] * PTESIZE);
		if(V(get_phys_mem32(pte)) == 0){
			print_error("V = 0", va, a, i, pte, old_pte);
			return -1;
		}
		if(R(get_phys_mem32(pte)) == 0 && W(get_phys_mem32(pte)) == 1){
			print_error("R = 0 & W = 1", va, a, i, pte, old_pte);
			return -1;
		}
		if(R(get_phys_mem32(pte)) == 1 || X(get_phys_mem32(pte)) == 1)
			break;
		i = i - 1;
		old_pte = pte;
		if(i < 0){
			print_error("cannot find R = 1 or X = 1 pte", va, a, i, pte, old_pte);
			return -1;
		}
		a = (get_phys_mem32(pte) >> 10) * PAGESIZE; 
	}
	//
	//check mstatus reg
	switch(mode){
		case PAGE_X:
			if(X(get_phys_mem32(pte)) == 0){
				print_error("this page is not excutable", va, a, i, pte, old_pte);
				return -1;
			}
			break;
		case PAGE_W:
			if(W(get_phys_mem32(pte)) == 0){
				print_error("this page is not writeable", va, a, i, pte, old_pte);
				return -1;
			}
			break;
		case PAGE_R:
			if(R(get_phys_mem32(pte)) == 0){
				print_error("this page is not readable", va, a, i, pte, old_pte);
				return -1;
			}
			break;
		default:
			break;
	}
	if(runlevel == U && U(get_phys_mem32(pte)) == 0){
		print_error("this page is not accessable by User mode", va, a, i, pte, old_pte);
		return -1;
	}
	if(MXR(csr[mstatus]) == 0 && X(get_phys_mem32(pte)) == 1){
		print_error("cannot excute X page when MXR = 0", va, a, i, pte, old_pte);
		return -1;
	}
	//
	ppn[1] = get_phys_mem32(pte) >> 20;
	ppn[0] = (get_phys_mem32(pte) >> 10) & 0x3ff;
	if(i > 0 && ppn[0] != 0){
		print_error("ppn error", va, a, i, pte, old_pte);
		return -1;
	}

	va_offset = pa_offset;
	if(i > 0)
		ppn[0] = vpn[0];
	pa = ppn[1] << 22 | ppn[0] << 12 | pa_offset;
	return pa;

}

void Emulator::print_error(string error, int32_t va, int32_t a, int i, int32_t pte, int32_t old_pte){
		cout << "page-fault exception : " << error << endl;
		cout << showbase << hex ;
		cout << "PC = "  << PC << endl;  
		cout << "va = " << va << endl;
	   	cout << "a = " << a << endl;
		cout << "satp = "  << csr[satp] << endl;
		cout << "i = " << i << endl;
	    cout << "pte = " << pte  << endl;
		if(i <= 0)
	    	cout << "old_pte = " << old_pte << endl ;
		cout << "get_phys_mem32(pte) = " << get_phys_mem32(pte) << endl ;
		if(i <= 0)
			cout << "get_phys_mem32(old_pte) = " << get_phys_mem32(old_pte) << endl ;
		cout << dec;
		dump_registers(0);
		//cout << "simulation halted" << endl;
}

