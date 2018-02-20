#ifndef EMULATOR_H_
#define EMULATOR_H_

#include<stdint.h>
#include<string>
#include<vector>
#include<stdio.h>
using namespace std;

#define REGISTERS_COUNT 32
#define DUMP_ROW 5

#define MEMSIZE 4*1024*1024

#define STARTPC 0
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
	uint32_t x[REGISTERS_COUNT];
	uint8_t* memory;
	int memsize = 0;
	uint32_t PC;
	vector<string> registers_name =
	{
		"x0","x1","x2","x3","x4","x5",
		"x6","x7","x8","x9","x10",
		"x11","x12","x13","x14","x15",
		"x16","x17","x18","x19","x20",
		"x21","x22","x23","x24","x25",
		"x26","x27","x28","x29","x30","x31"
	};


	Emulator(){
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
		return (memory[PC+3] << 24)|(memory[PC+2] << 16)|(memory[PC+1] << 8)|(memory[PC]);
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

	void dump_memory(size_t limit){
		int i;
		for(i=0;i<limit;i++){
			if(i % 5 == 0)	printf("\n");
			printf("%08x:%02x	",i,memory[i]);
		}
		printf("\n");
	}

};

#endif
