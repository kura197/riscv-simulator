#include <iostream>
#include <fstream>
#include <string.h>
#include "emulator.hpp"
#include "instruction.hpp"
#include "csr.hpp"
#include "reg.h"

#define MASK_OPCODE 0x0000007f
#define RD(x) (x >> 7) & 0b11111
#define FUNCT3(x) (x >> 12) & 0b111
#define RS1(x) (x  >> 15) & 0b11111
#define RS2(x) (x >> 20) & 0b11111
#define FUNCT7(x) (x >> 25) & 0b1111111
#define CSR(x) (x >> 20) & 0xfff

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
	instruction[0b1110011] = OP_SYSTEM;
	instruction[0b0101111] = OP_A;
}


decoder_t decode(uint32_t instr){
	decoder_t d = {};
	d.opcode = instr & MASK_OPCODE;
	d.rd = RD(instr);
	d.funct3 = FUNCT3(instr);
	d.rs1 = RS1(instr);
	d.rs2 = RS2(instr);
	d.funct7 = FUNCT7(instr);
	d.csr = CSR(instr);
	
	int r = (d.funct3 == 0b001 || d.funct3 == 0b101) ? 1 : 0;
	char type;
	if(d.opcode == 0b0110011 || (d.opcode == 0b0010011 && r)){
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
				d.imm = 0xfffff000 | ((instr & 0xfe000000) >> 20) | ((instr & 0x00000f80) >> 7);
			else
				d.imm = ((instr & 0xfe000000) >> 20) | ((instr & 0x00000f80) >> 7);
			break;
		case 'B':
			if(instr >> 31)
				d.imm = 0xfffff000 | ((instr & 0xfe000000) >> 20) | ((instr & 0x00000f00) >> 7) | ((instr & 0x00000080) << 4);
			else
				d.imm = ((instr & 0xfe000000) >> 20) | ((instr & 0x00000f00) >> 7) | ((instr & 0x00000080) << 4);
			break;
		case 'U':
			d.imm = instr & 0xfffff000;
			break;
		case 'J':
			if(instr >> 31)
				d.imm = 0xfff00000 | (instr & 0x000ff000) | ((instr & 0x00100000) >> 9) | ((instr & 0x7fe00000) >> 20);
			else
				d.imm = (instr & 0x000ff000) | ((instr & 0x00100000) >> 9) | ((instr & 0x7fe00000) >> 20);
			break;
		default:
			//cout << "unknown opcode" << endl;
			break;
	}

	return d;
}

void OP_LUI(Emulator* emu, decoder_t d){
	emu->x[d.rd] = 0xfffff000 & d.imm;
#ifdef DEBUG
	printf("rd = %d, imm = %08x(%d)\n", d.rd, d.imm, d.imm);
#endif
}

void OP_AUIPC(Emulator* emu, decoder_t d){
	emu->x[d.rd] = (0xfffff000 & d.imm) + (emu->PC - 4);
#ifdef DEBUG
	printf("rd = %d, imm = %08x(%d), PC = %08x\n", d.rd, d.imm, d.imm, (emu->PC - 4));
#endif
}

void OP_JAL(Emulator* emu, decoder_t d){
	emu->x[d.rd] = emu->PC;
	emu->PC += d.imm - 4;
#ifdef DEBUG
	printf("Jump!! rd = %d, imm = %08x(%d), PC = %08x\n", d.rd, d.imm, d.imm, emu->PC);
#endif
}

void OP_JALR(Emulator* emu, decoder_t d){
	emu->x[d.rd] = emu->PC;
	emu->PC = (d.imm + emu->x[d.rs1]) & 0xfffffffe;
#ifdef DEBUG
	printf("Jump!! rd = %d, rs1 = %d, imm = %08x(%d), PC = %08x\n", d.rd, d.rs1, d.imm, d.imm, emu->PC);
#endif
}

void OP_B(Emulator* emu, decoder_t d){
	switch(d.funct3){
		//BEQ
		case 0b000:
			if(emu->x[d.rs1] == emu->x[d.rs2])
				emu->PC += d.imm - 4;
			break;
		//BNE
		case 0b001:
			if(emu->x[d.rs1] != emu->x[d.rs2])
				emu->PC += d.imm - 4;
			break;
		//BLT
		case 0b100:
			if(emu->x[d.rs1] < emu->x[d.rs2])
				emu->PC += d.imm - 4;
			break;
		//BGE
		case 0b101:
			if(emu->x[d.rs1] >= emu->x[d.rs2])
				emu->PC += d.imm - 4;
			break;
		//BLTU
		case 0b110:
			if((uint32_t)emu->x[d.rs1] < (uint32_t)emu->x[d.rs2])
				emu->PC += d.imm - 4;
			break;
		//BGEU
		case 0b111:
			if((uint32_t)emu->x[d.rs1] >= (uint32_t)emu->x[d.rs2])
				emu->PC += d.imm - 4;
			break;
		default:
			cout << "error : OP_B/no match op" << endl;
			break;
	}
#ifdef DEBUG
	printf("Jump!! rd = %d, rs1 = %d, imm = %08x(%d)\n, PC = %08x", d.rd, d.rs1, d.imm, d.imm, emu->PC);
#endif
}

void OP_I(Emulator* emu, decoder_t d){
	switch(d.funct3){
		//LB
		case 0b000:
			{
			int8_t mem = emu->get_mem8(emu->x[d.rs1] + d.imm);
			if(mem >> 7)
				emu->x[d.rd] = mem | 0xffffff00;
			else
				emu->x[d.rd] = mem & 0x000000ff;
			break;
			}
		//LH
		case 0b001:
			{
			int16_t mem = emu->get_mem16(emu->x[d.rs1] + d.imm);
			if(mem >> 15)
				emu->x[d.rd] = mem | 0xffff0000;
			else
				emu->x[d.rd] = mem & 0x0000ffff;
			break;
			}
		//LW
		case 0b010:
			emu->x[d.rd] = emu->get_mem32(emu->x[d.rs1] + d.imm);
			break;
		//LBU
		case 0b100:
			emu->x[d.rd] = emu->get_mem8(emu->x[d.rs1] + d.imm) & 0x000000ff;
			break;
		//LHU
		case 0b101:
			emu->x[d.rd] = emu->get_mem16(emu->x[d.rs1] + d.imm) & 0x0000ffff;
			break;
		//unknown
		default:
				cout << "error : OP_I/no match op" << endl;
				break;
	}
#ifdef DEBUG
	printf("funct3 = %d, rd = %d, rs1 = %d, rs2 = %d, imm = %08x(%d)\n",d.funct3, d.rd, d.rs1, d.rs2, d.imm, d.imm);
#endif
}

void OP_S(Emulator* emu, decoder_t d){
	switch(d.funct3){
		//SB
		case 0b000:
			emu->store_mem8(emu->x[d.rs1] + d.imm, emu->x[d.rs2]);
			break;
		//SH
		case 0b001:
			emu->store_mem16(emu->x[d.rs1] + d.imm, emu->x[d.rs2]);
			break;
		//SW
		case 0b010:
			emu->store_mem32(emu->x[d.rs1] + d.imm, emu->x[d.rs2]);
			break;
		//unknown
		default:
			cout << "error : OP_S/no match op" << endl;
			break;
	}
#ifdef DEBUG
	printf("funct3 = %d, rd = %d, rs1 = %d, rs2 = %d, imm = %08x(%d)\n",d.funct3, d.rd, d.rs1, d.rs2, d.imm, d.imm);
#endif

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
			emu->x[d.rd] = emu->x[d.rs1] << d.rs2;
			break;
		//SRLI SRAI
		case 0b101:
			//SRLI
			if(d.funct7 == 0b0000000)
				emu->x[d.rd] = (uint32_t)emu->x[d.rs1] >> d.rs2;
			//SRAI
			else if(d.funct7	== 0b0100000)
				emu->x[d.rd] = emu->x[d.rs1] >> d.rs2;
			else
				cout << "error : OP_IR/SRLI_SRAI FUNCT7 erorr" << endl;
			break;
		//unknown
		default:
				cout << "error : OP_IR/no match op" << endl;
				break;
	}
#ifdef DEBUG
	printf("funct3 = %d, rd = %d, rs1 = %d, imm = %08x(%d)\n",d.funct3, d.rd, d.rs1, d.imm, d.imm);
#endif
}

void OP_R(Emulator* emu, decoder_t d){
	//I
	if(d.funct7 == 0b0000000 || d.funct7 == 0b0100000){
		switch(d.funct3){
			//ADD/SUB
			case 0b000:
				//ADD
				if(d.funct7 == 0b0000000)
					emu->x[d.rd] = emu->x[d.rs1] + emu->x[d.rs2];
				//SUB
				else
					emu->x[d.rd] = emu->x[d.rs1] - emu->x[d.rs2];
				break;
				//SLL
			case 0b001:
				emu->x[d.rd] = emu->x[d.rs1] <<(emu->x[d.rs2] & 0b11111);
				break;
				//SLT
			case 0b010:
				emu->x[d.rd] = (emu->x[d.rs1] < emu->x[d.rs2]) ? 1 : 0;
				break;
				//SLTU
			case 0b011:
				emu->x[d.rd] = ((uint32_t)emu->x[d.rs1] < (uint32_t)emu->x[d.rs2]) ? 1 : 0;
				break;
				//XOR
			case 0b100:
				emu->x[d.rd] = emu->x[d.rs1] ^ emu->x[d.rs2];
				break;
				//SRL/SRA
			case 0b101:
				//SRL
				if(d.funct7 == 0b0000000)
					emu->x[d.rd] = (uint32_t)emu->x[d.rs1] >> (emu->x[d.rs2] & 0b11111);
				//SRA
				else
					emu->x[d.rd] = emu->x[d.rs1] >> (emu->x[d.rs2] & 0b11111);
				break;
				//OR
			case 0b110:
				emu->x[d.rd] = emu->x[d.rs1] | emu->x[d.rs2];
				break;
				//AND
			case 0b111:
				emu->x[d.rd] = emu->x[d.rs1] & emu->x[d.rs2];
				break;
				//unknown
			default:
				cout << "error : OP_R/no match op" << endl;
				break;
		}
	//ISA-M
	}else if(d.funct7 == 0b0000001){
		switch(d.funct3){
			//MUL
			case 0b000:
				emu->x[d.rd] = emu->x[d.rs1] * emu->x[d.rs2];
				break;
			//MULH
			case 0b001:
				emu->x[d.rd] = (int32_t)(((int64_t)emu->x[d.rs1] * (int64_t)emu->x[d.rs2]) >> 32);
				break;
			//MULHSU
			case 0b010:
				emu->x[d.rd] = (int32_t)(((int64_t)emu->x[d.rs1] * (uint64_t)emu->x[d.rs2]) >> 32);
				break;
			//MULHU
			case 0b011:
				emu->x[d.rd] = (int32_t)(((uint64_t)emu->x[d.rs1] * (uint64_t)emu->x[d.rs2]) >> 32);
				break;
			//DIV
			case 0b100:
				emu->x[d.rd] = emu->x[d.rs1] / emu->x[d.rs2];
				break;
			//DIVU
			case 0b101:
				emu->x[d.rd] = (uint32_t)emu->x[d.rs1] / (uint32_t)emu->x[d.rs2];
				break;
			//REM
			case 0b110:
				emu->x[d.rd] = emu->x[d.rs1] % emu->x[d.rs2];
				break;
			//REMU
			case 0b111:
				emu->x[d.rd] = (uint32_t)emu->x[d.rs1] % (uint32_t)emu->x[d.rs2];
				break;
			default:
				cout << "error : OP-R/ISA-M no match op" << endl;
		}
	}else{
		cout << "error : OP_R/funct7 error" << endl;
	}
#ifdef DEBUG
	printf("funct7 = %d, funct3 = %d, rd = %d, rs1 = %d, rs2 = %d, imm = %08x(%d)\n",d.funct7, d.funct3, d.rd, d.rs1, d.rs2, d.imm, d.imm);
#endif

}

void OP_SYSTEM(Emulator* emu, decoder_t d){
	//if(num2csr == 0){
	if(emu->runlevel == 0 && !(d.funct3 == 0b000 && d.funct7 == 0)){
		cout << "illegal privilleged level" << endl;
		return;
	}
	uint8_t uimm = d.rs1;
	switch(d.funct3){
		//ECALL / MRET 
		case 0b000:
			//ECALL
			if(d.funct7 == 0 && d.rs1 == 0 && d.rd == 0){
				int32_t ex_code = (emu->runlevel == U) ? 8 : 11;
				emu->csr[mcause] = ex_code;
				emu->csr[mstatus] |= emu->runlevel << 11 ;
				emu->runlevel = M;
				//PC should be PC - 4
				//emu->csr[mepc] = emu->V2P(emu->PC, -1);
				emu->csr[mepc] = emu->PC;
				emu->PC = emu->get_mem32(BASE(emu->csr[mtvec]));
				emu->csr[mstatus] |= MIE(emu->csr[mstatus]) << 7 ;
				//disable interrupt ??
				emu->csr[mstatus] &= 0xfffffff7;
				//int32_t tmp = emu->x[2];
				//emu->x[2] = emu->csr[mscratch];
				//emu->csr[mscratch] = tmp;
				//MRET
			}else if(d.funct7 == 0x18 && d.rs1 == 0 && d.rd == 0){
				emu->PC = emu->csr[mepc];
				emu->runlevel = MPP(emu->csr[mstatus]);
				//MPP(emu->csr[mcause]) = U;
				//MIE(emu->csr[mcause]) = MPIE(emu->csr[mcause]);
				//MPIE(emu->csr[mcause]) = 1;
				emu->csr[mstatus] |= U << 11 ;
				emu->csr[mstatus] |= MPIE(emu->csr[mstatus]) << 3 ;
				emu->csr[mstatus] |= 1 << 7;
				//int32_t tmp = emu->x[2];
				//emu->x[2] = emu->csr[mscratch];
				//emu->csr[mscratch] = tmp;
			}else{
				cout << "error : SYSTEM_OP/funct3 == 000" << endl;
			}
			break;
		//CSRRW
		case 0b001:
			emu->x[d.rd] = emu->csr[num2csr(d.csr, emu->runlevel)];
			emu->csr[num2csr(d.csr, emu->runlevel)] = emu->x[d.rs1];
			break;
		//CSRRS
		case 0b010:
			emu->x[d.rd] = emu->csr[num2csr(d.csr, emu->runlevel)];
			emu->csr[num2csr(d.csr, emu->runlevel)] |= emu->x[d.rs1];
			break;
		//CSRRC
		case 0b011:
			emu->x[d.rd] = emu->csr[num2csr(d.csr, emu->runlevel)];
			emu->csr[num2csr(d.csr, emu->runlevel)] &= ~emu->x[d.rs1];
			break;
		//CSRRWI
		case 0b101:
			emu->x[d.rd] = emu->csr[num2csr(d.csr, emu->runlevel)];
			emu->csr[num2csr(d.csr, emu->runlevel)] = uimm;
			break;
		//CSRRSI
		case 0b110:
			emu->x[d.rd] = emu->csr[num2csr(d.csr, emu->runlevel)];
			emu->csr[num2csr(d.csr, emu->runlevel)] |= uimm;
			break;
		//CSRRCI
		case 0b111:
			emu->x[d.rd] = emu->csr[num2csr(d.csr, emu->runlevel)];
			emu->csr[num2csr(d.csr, emu->runlevel)] &= ~uimm;
			break;
		default:
			cout << "error : OP_SYSTEM/no match funst3" << endl;
	}
}

void OP_A(Emulator* emu, decoder_t d){
	switch((d.funct7 >> 2) & 0b11111){
		//AMOSWAP.W
		case 0b00001:
			emu->x[d.rd] = emu->get_mem32(emu->x[d.rs1]);
			emu->store_mem32(emu->x[d.rs1], emu->x[d.rs2]);
			//emu->x[d.rs2] = emu->x[d.rd];
			break;
		default:
			cout << "error : OP_A/not yet implemented" << endl;
	}
}
