#include <iostream>
#include <fstream>
#include <string.h>
#include "emulator.hpp"
#include "instruction.hpp"

#define MASK_OPCODE 0x0000007f
#define RD(x) (x >> 7) & 0b11111
#define FUNCT3(x) (x >> 12) & 0b111
#define RS1(x) (x  >> 15) & 0b11111
#define RS2(x) (x >> 20) & 0b11111
#define FUNCT7(x) (x >> 25) & 0b1111111

using namespace std;

instruction_func_t* instruction[128];

void init_instruction(){
	memset(instruction,0,sizeof(instruction));

	instruction[0b0110111] = OP_LUI;
	instruction[0b0010111] = OP_AUIPC;
	instruction[0b1101111] = OP_JAL;
	instruction[0b1100111] = OP_JALR;
	instruction[0b1100011] = OP_B;
	instruction[0b0000011] = OP_I;
	instruction[0b0100011] = OP_S;
	instruction[0b0010011] = OP_IR;
	instruction[0b0110011] = OP_R;
}


decoder_t decode(uint32_t instr){
	decoder_t d = {};
	d.opcode = instr & MASK_OPCODE;
	d.rd = RD(instr);
	d.funct3 = FUNCT3(instr);
	d.rs1 = RS1(instr);
	d.rs2 = RS2(instr);
	d.funct7 = FUNCT7(instr);
	
	int r = (d.funct3 == 0b000 || d.funct3 == 0b001 || d.funct3 == 0b010 || d.funct3 == 0b100 || d.funct3 == 0b101) ? 1 : 0;
	char type;
	if(d.opcode == 0b0110011 || (d.opcode == 0b0010011 && !r)){
		type = 'R';
	}else if(d.opcode == 0b0000011 || d.opcode == 0b0010011 || d.opcode == 0b1100111){
		type = 'I';
	}else if(d.opcode == 0b0100011){
		type = 'S';
	}else if(d.opcode == 0b1100011){
		type = 'B';
	}else if(d.opcode == 0b0110111 || d.opcode == 0b0010111){
		type = 'U';
	}else if(d.opcode == 0b1101111){
		type = 'J';
	}else{
		type = '?';
	}

	switch(type){
		case 'R':
			break;
		case 'I':
			if(instr >> 31)
				d.imm = 0xfffff000 | ((instr & 0xfff00000) >> 20);
			else
				d.imm = (instr & 0xfff00000) >> 20;
			break;
		case 'S':
			if(instr >> 31)
				d.imm = 0xfffff000 | ((instr & 0xfe000000) >> 25) | ((instr & 0x00000f80) >> 7);
			else
				d.imm = ((instr & 0xfe000000) >> 25) | ((instr & 0x00000f80) >> 7);
			break;
		case 'B':
			if(instr >> 31)
				d.imm = 0xfffff000 | ((instr & 0xfe000000) >> 25) | ((instr & 0x00000f00) >> 8) | ((instr & 0x00000080) << 5);
			else
				d.imm = ((instr & 0xfe000000) >> 25) | ((instr & 0x00000f00) >> 8) | ((instr & 0x00000080) << 5);
			break;
		case 'U':
			d.imm = instr & 0xfffff000;
			break;
		case 'J':
			if(instr >> 31)
				d.imm = 0xfff00000 | (instr & 0x0007f000) | ((instr & 0x00100000) >> 10) | ((instr & 0x7fe00000) >> 21);
			else
				d.imm = (instr & 0x0007f000) | ((instr & 0x00100000) >> 10) | ((instr & 0x7fe00000) >> 21);
			break;
		default:
			cout << "unknown opcode" << endl;
			break;
	}

	return d;
}

void OP_LUI(Emulator* emu, decoder_t d){

}

void OP_AUIPC(Emulator* emu, decoder_t d){

}

void OP_JAL(Emulator* emu, decoder_t d){

}

void OP_JALR(Emulator* emu, decoder_t d){

}

void OP_B(Emulator* emu, decoder_t d){

}

void OP_I(Emulator* emu, decoder_t d){

}

void OP_S(Emulator* emu, decoder_t d){

}

void OP_IR(Emulator* emu, decoder_t d){
	switch(d.funct3){
		//ADDI
		case 0b000:
			emu->x[d.rd] = emu->x[d.rs1] + d.imm;
			break;
		//SLTI
		case 0b010:
			emu->x[d.rd] = (emu->x[d.rs1] < d.imm) ? 1 : 0;
			break;
		//SLTIU
		case 0b011:
			emu->x[d.rd] = (emu->x[d.rs1] < (uint32_t)d.imm) ? 1 : 0;
			break;
		//XORI
		case 0b100:
			emu->x[d.rd] = emu->x[d.rs1] ^ d.imm;
			break;
		//ORI
		case 0b110:
			emu->x[d.rd] = emu->x[d.rs1] | d.imm;
			break;
		//ANDI
		case 0b111:
			emu->x[d.rd] = emu->x[d.rs1] & d.imm;
			break;
		//SLLI
		case 0b001:
			emu->x[d.rd] = emu->x[d.rs1] << (d.imm & 0b11111);
			break;
		//SRLI SRAI
		case 0b101:
			//SRLI
			if(d.funct7 == 0b0000000)
				emu->x[d.rd] = emu->x[d.rs1] >> (d.imm & 0b11111);
			//SRAI
			else if(d.funct7	== 0b0100000)
				emu->x[d.rd] = (d.imm >> 31) ? (emu->x[d.rs1] >> (d.imm & 0b11111)) | 0xffffff << (32 - d.imm & 0b11111)
												: emu->x[d.rs1] >> (d.imm & 0b11111);
			else
				cout << "error : OR_IR/SRLI_SRAI FUNCT7 erorr" << endl;
			break;
		//unknown
		default:
				cout << "error : OR_IR/no match op" << endl;
				break;
	}
}

void OP_R(Emulator* emu, decoder_t d){

}



