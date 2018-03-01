#include <string>
#include <iostream>
#include <fstream>
#include <gflags/gflags.h>
#include "emulator.hpp"
#include "instruction.hpp"
#include "trap.hpp"
using namespace std;

#define SECTSIZE 512

DEFINE_bool(d, false, "set debug flag");

#define DEBUG

void ioport(Emulator* emu, ifstream *binary);

int main(int argc, char* argv[]){
	if(argc==1){
		cout<<"no input binary" << endl;
		return 1;
	}
	else if(argc>2){
		cout<< "too many input files." << endl;
		return 1;
	}
	ifstream binary(argv[1],ios::in|ios::binary);
	if(!binary){
		cout << "can not open binary file" << endl;
		return 1;
	}
	Emulator emu;
	emu.load_memory(&binary, 0x0, 0x7c00, 0x200);
	//emu.load_memory(&binary, 0x200, 0x10000, 4096);
	//emu.dump_memory(0x10000,0x100);
	init_instruction();
	//emu.dump_registers();
	//emu.dump_memory(0x7c00, 64);
	uint32_t instr;
	while(emu.PC < MEMSIZE){
		interrupt(&emu);
		instr = emu.fetch();
		//finish
		if(instr == 0x00000000)
			break;
#ifdef DEBUG
		cout << endl;
		emu.dump_registers(0);
		//emu.dump_memory(0x10000,16);
		printf("PC = %08x, Code = %08x, runlevel = %d\n",emu.V2P(emu.PC-4),instr,emu.runlevel);
#endif
		//instruction
		decoder_t d;
		d = decode(instr);
		instruction[d.opcode](&emu,d);
		ioport(&emu, &binary);
	}
	emu.dump_registers(0);
	printf("program ended successfully at PC = %08x\n", emu.V2P(emu.PC-4));
	//emu.dump_memory(0x10000,0x20);

	binary.close();
	return 0;
}

void ioport(Emulator* emu, ifstream *binary){

/*  READ DISK DATA  */
//0x1F3:	read op
//0x1F4 - 0x1F7:	phisical address
//0x1F8 - 0x1FB:	read address(offset)
//0x1FC - 0x1FF:	read size
	if(emu->memory[0x1F3] == 1){
		emu->memory[0x1F3] = 0;
		uint32_t pa = emu->get_mem32(0x1F4);
		uint32_t offset = emu->get_mem32(0x1F8) * SECTSIZE;
		uint32_t size = emu->get_mem32(0x1FC);
		emu->dump_memory(0x1F4,12);
		emu->load_memory(binary, offset, pa, size);
	}
}


