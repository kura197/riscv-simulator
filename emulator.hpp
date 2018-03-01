#ifndef EMULATOR_H_
#define EMULATOR_H_

#include<stdint.h>
#include<string>
#include<vector>
#include<stdio.h>
#include<iostream>
#include<fstream>
#include "csr.hpp"
#include "reg.h"
using namespace std;


#define USER_REG_CNT 32
#define DUMP_ROW 8

#define MEMSIZE 256 * 1024 * 1024

#define STARTPC 0x7c00
#define RESET_VECTOR 0
#define FIRST_SP 0x0800

#define PAGESIZE 4096
#define LEVELS 2
#define PTESIZE 4;

enum PLEVEL{U, S, R, M};

typedef struct decoder{
	uint8_t opcode;
	uint8_t rd,rs1;
	uint8_t rs2;
	uint8_t funct3,funct7;
	int32_t imm;
	uint16_t csr;
}decoder_t;


class Emulator{
	public:
	int32_t x[USER_REG_CNT];
	int32_t csr[CSR_CNT];
	uint8_t* memory;
	uint32_t PC;
	int8_t runlevel;

	Emulator(){
		clear_registers();
		//always 0
		x[0] = 0;
		//stack pointer
		x[2] = FIRST_SP;
		PC = STARTPC;
		runlevel = M;
		memory = new uint8_t[MEMSIZE];
	}

	~Emulator(){
		delete memory;
	}

	void load_memory(ifstream *binary, uint32_t bin_addr, uint32_t mem_addr, int size){
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

	void clear_registers(){
		for(int i=0;i<USER_REG_CNT;i++)
			x[i] = 0;
		for(int i=0;i<CSR_CNT;i++)
			csr[i] = 0;
	}

	uint32_t fetch(){
		uint32_t instr;
		instr = (memory[V2P(PC+3)] << 24)|(memory[V2P(PC+2)] << 16)|(memory[V2P(PC+1)] << 8)|(memory[V2P(PC)]);
		x[0] = 0;
		PC += 4;
		return instr;
	}

	void dump_registers(int num){
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

	void dump_memory(int32_t start_addr, size_t limit){
		int i, k;
		for(i = start_addr,k = 0;i < start_addr + limit; i++, k++){
			if(k % DUMP_ROW == 0)	printf("\n");
			printf("%08x:%02x	",i,memory[i]);
		}
		printf("\n");
	}

	int32_t get_mem32(uint32_t addr){
		return (memory[V2P(addr+3)] << 24)|(memory[V2P(addr+2)] << 16)|(memory[V2P(addr+1)] << 8)|(memory[V2P(addr)]);
	}

	int16_t get_mem16(uint32_t addr){
		return (memory[V2P(addr+1)] << 8)|(memory[V2P(addr)]);
	}

	int8_t get_mem8(uint32_t addr){
		return memory[V2P(addr)];
	}

	void store_mem32(uint32_t addr, int32_t value){
		for(int i = 0; i < 4; i++){
			memory[V2P(addr)] = (value >> 8*i & 0xff);
			addr++;
		}
	}

	void store_mem16(uint32_t addr, int16_t value){
		for(int i = 0; i < 2; i++){
			memory[V2P(addr)] = (value >> 8*i & 0xff);
			addr++;
		}
	}

	void store_mem8(uint32_t addr, int8_t value){
		memory[V2P(addr)] = value;
	}

	//mimpic(external interrupt)
	uint8_t read_exinterrupt(){
		uint8_t ex = (csr[mimpid] & 0xff);
		return ex;
	}

	void set_exinterrupt(int8_t num){
		csr[mimpid] = num;
	}

	uint32_t V2P(uint32_t va){
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
		while(1){
			pte = (uint32_t)(a + vpn[i] * PAGESIZE);
			if(V(memory[pte]) == 0 || (R(memory[pte]) == 0 && W(memory[pte]) == 1)){
				cout << "page-fault exception" << endl;
				return 0;
			}
			if(R(memory[pte]) == 1 || X(memory[pte]) == 1)
				break;
			i = i - 1;
			if(i < 0){
				cout << "page-fault exception" << endl;
				return 0;
			}
			a = (memory[pte] >> 10) * PAGESIZE; 
		}
		//
		//check mstatus reg
		//
		ppn[1] = memory[pte] >> 20;
		ppn[0] = memory[pte] >> 10 & 0x3ff;
		if(i > 0 && ppn[0] != 0){
			cout << "page-fault exception" << endl;
			return 0;
		}

		va_offset = pa_offset;
		if(i > 0)
			ppn[0] = vpn[0];
		pa = ppn[1] << 22 | ppn[0] << 12 | pa_offset;
		return pa;
	}

};

#endif
