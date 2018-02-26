#ifndef INSTRUCTION_H_
#define INSTRUCTION_H_

#include "emulator.hpp"

decoder_t decode(uint32_t instr);
void init_instruction();

void OP_LUI(Emulator* emu, decoder_t d);
void OP_AUIPC(Emulator* emu, decoder_t d);
void OP_JAL(Emulator* emu, decoder_t d);
void OP_JALR(Emulator* emu, decoder_t d);
void OP_B(Emulator* emu, decoder_t d);
void OP_I(Emulator* emu, decoder_t d);
void OP_S(Emulator* emu, decoder_t d);
void OP_IR(Emulator* emu, decoder_t d);
void OP_R(Emulator* emu, decoder_t d);
void OP_SYSTEM(Emulator* emu, decoder_t d);

typedef void instruction_func_t(Emulator*, decoder_t);
extern instruction_func_t* instruction[128];

#endif
