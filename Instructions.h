#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <vector>

// MIPS instructions
enum Opcode {
    ADD, ADDI, SUB, MUL, AND_OP, OR_OP, SLL, SRL, LW, SW, BEQ, J, NOP_OP
};

// single decoded MIPS instruction
struct Instructions {
    Opcode op;
    int rs;
    int rt;
    int rd;
    int imm;
    int address;
};

// returns a NOP used to fill empty pipeline stages
Instructions makeNOP();

#endif