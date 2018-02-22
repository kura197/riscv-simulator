#include<string>
#include<iostream>
#include<fstream>
#include"emulator.hpp"
#include"instruction.hpp"
using namespace std;


#define MEMORY_SIZE (4*5)


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
	emu.read_memory(&binary);
	binary.close();
	//emu.clear_registers();
	init_instruction();
	//emu.dump_registers();
	//emu.dump_memory(emu.memsize);
	//????
	emu.memsize -= 4;
	//
	while(emu.PC < emu.memsize){
		uint32_t instr = emu.fetch();
		//finish
		if(instr == 0x00000000)
			break;
#ifdef DEBUG
		emu.dump_registers();
		//emu.dump_memory(0xf0,8);
		printf("PC = %08x, Code = %08x\n",emu.PC-4,instr);
#endif
		//instruction
		decoder_t d;
		d = decode(instr);
		instruction[d.opcode](&emu,d);
	}
	emu.dump_registers();
	emu.dump_memory(0xf0,8);

	return 0;
}


