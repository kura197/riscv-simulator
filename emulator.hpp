#ifndef EMULATOR_H_
#define EMULATOR_H_

#include<stdint.h>
#include<string>
#include<vector>
#include<stdio.h>
using namespace std;

//#define DEBUG

#define REGISTERS_COUNT 32
#define DUMP_ROW 5

#define MEMSIZE 4*1024*1024

#define STARTPC 0x00
#define RESET_VECTOR 0
#define FIRST_SP 1024

typedef struct decoder{
	uint8_t opcode;
	uint8_t rd,rs1;
	uint8_t rs2;
	uint8_t funct3,funct7;
	int32_t imm;
}decoder_t;


class Emulator{
	public:
	int32_t x[REGISTERS_COUNT];
	uint8_t* memory;
	int memsize = 0;
	uint32_t PC;

	Emulator(){
		clear_registers();
		//always 0
		x[0] = 0;
		//stack pointer
		x[2] = FIRST_SP;
		PC = STARTPC;
		memory = new uint8_t[MEMSIZE];
	}

	~Emulator(){
		delete memory;
	}

	void read_memory(ifstream *binary){
		uint8_t* memory_ptr = memory;
		while(!binary->eof()){
			binary->read((char*)memory_ptr, sizeof(uint8_t));
			memory_ptr++;
			memsize++;
		}
	}

	void clear_registers(){
		for(int i=0;i<REGISTERS_COUNT;i++)
			x[i] = 0;
	}

	uint32_t fetch(){
		uint32_t instr;
		instr = (memory[PC+3] << 24)|(memory[PC+2] << 16)|(memory[PC+1] << 8)|(memory[PC]);
		x[0] = 0;
		PC += 4;
		return instr;
	}

	void dump_registers(){
		int i;
		for(i=0;i<REGISTERS_COUNT;i++){
			if(i % DUMP_ROW == 0)	printf("\n");
			if(i < 10)
				printf(" x%d=%08x	",i,x[i]);
			else
				printf("x%d=%08x	",i,x[i]);
		}
		printf("\n");
	}

	void dump_memory(int32_t start_addr, size_t limit){
		for(int i = start_addr;i < start_addr + limit; i++){
			if(i % 5 == 0)	printf("\n");
			printf("%08x:%02x	",i,memory[i]);
		}
		printf("\n");
	}

	int32_t get_mem32(int32_t addr){
		return (memory[addr+3] << 24)|(memory[addr+2] << 16)|(memory[addr+1] << 8)|(memory[addr]);
	}

	int16_t get_mem16(int32_t addr){
		return (memory[addr+1] << 8)|(memory[addr]);
	}

	int8_t get_mem8(int32_t addr){
		return memory[addr];
	}

	void store_mem32(int32_t addr, int32_t value){
		for(int i = 0; i < 4; i++){
			memory[addr] = (value >> 8*i & 0xff);
			addr++;
		}
	}

	void store_mem16(int32_t addr, int16_t value){
		for(int i = 0; i < 2; i++){
			memory[addr] = (value >> 8*i & 0xff);
			addr++;
		}
	}

	void store_mem8(int32_t addr, int8_t value){
		memory[addr] = value;
	}

};

#endif
