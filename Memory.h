#ifndef MEMORY_H
#define MEMORY_H

const int NUM_REGS = 32; // 32 general purpose registers
const int MEM_SIZE = 1024; // mem size in words

extern int R[NUM_REGS]; // register file
extern int MEM[MEM_SIZE]; // main memory

void initRegisters(); // zero out all regiseters
void initMemory(); // zero out all memory

#endif