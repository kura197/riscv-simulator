#ifndef TRAP_H_
#define TRAP_H_

#include "emulator.hpp"

void interrupt(Emulator*);
void check_interrupt(Emulator*, int8_t, int32_t, int32_t, int32_t);

#endif
