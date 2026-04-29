#include "Memory.h"

int R[NUM_REGS];
int MEM[MEM_SIZE];

// zero out all registers
void initRegisters() {
    for(int i = 0; i < NUM_REGS; i++) {
        R[i] = 0;
    }
}

// zero out all memory
void initMemory() {
    for(int i = 0; i < MEM_SIZE; i++) {
        MEM[i] = 0;
    }
}