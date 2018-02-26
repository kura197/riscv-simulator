#include <string>
#include <iostream>
#include <fstream>
#include "emulator.hpp"
#include "instruction.hpp"
#include "trap.hpp"
using namespace std;


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
	emu.read_memory(&binary, 0x7c00);
	binary.close();
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
		//emu.dump_memory(0xf0,8);
		printf("PC = %08x, Code = %08x, runlevel = %d\n",emu.V2P(emu.PC-4),instr,emu.runlevel);
#endif
		//instruction
		decoder_t d;
		d = decode(instr);
		instruction[d.opcode](&emu,d);
	}
	emu.dump_registers(0);
	emu.dump_memory(0xf0,8);

	return 0;
}


