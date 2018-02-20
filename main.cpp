#include<string>
#include<iostream>
#include<fstream>
#include"emulator.hpp"
#include"instruction.hpp"
using namespace std;

#define DEBUG

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
	emu.clear_registers();
	init_instruction();
	//emu.dump_registers();
	emu.dump_memory(emu.memsize);
	//????
	emu.memsize -= 4;
	//
	while(emu.PC < emu.memsize){
		uint32_t instr = emu.fetch();
#ifdef DEBUG
		emu.dump_registers();
		printf("PC = %08x, Code = %08x\n",emu.PC,instr);
#endif
		//instruction
		decoder_t d;
		d = decode(instr);
		instruction[d.opcode](&emu,d);
		emu.PC += 4;
		emu.x[0] = 0;
	}
	//init_instruction();
	/*
	while(emu->eip < MEMORY_SIZE){
		uint8_t code = get_code8(emu,0);
		printf("EIP = %X, Code = %02X\n", emu->eip, code);
		if(instruction[code] == NULL){
			printf("\n\nnot implemented : %x\n\n",code);
			break;
		}
		instruction[code](emu);
		if(emu->eip==0x00){
			printf("\n\nend of program\n\n");
			break;
		}
	}	
	dump_registers(emu);
	destroy_emu(emu);
	*/
	return 0;
}


