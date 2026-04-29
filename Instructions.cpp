#include "Instructions.h"

// NOP instruction for filling empty pipeline stages
Instructions makeNOP() {
    Instructions nop;

    nop.op = NOP_OP;
    nop.rs = 0;
    nop.rt = 0;
    nop.rd = 0;
    nop.imm = 0;
    nop.address = 0;

    return nop;
}