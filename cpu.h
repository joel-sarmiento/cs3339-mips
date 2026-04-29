#ifndef CPU_H
#define CPU_H

#include <vector>
#include "Instructions.h"
using namespace std;

// if/id reg: holds fethced instruction and current PC
struct IFID {
    Instructions instr;
    int pc;
    bool nop;
};

// id/ex reg: holds decoded instruction, reg values, and immediate
struct IDEX {
    Instructions instr;
    int pc;
    int rsVal; // value read from rs reg
    int rtVal; // value read from rt reg
    int imm;
    bool nop;
};

// ex/mem reg: holds ALU result and branch info
struct EXMEM {
    Instructions instr;
    int aluResult;
    int rtVal; // rt value forwarded for SW
    bool takeBranch; // true if branch is taken
    int branchTarget; // branch target address
    bool nop;
};

// mem/wb reg: holds data from memory stage to be written back
struct MEMWB {
    Instructions instr;
    int memData; // data loaded from memory (LW)
    int aluResult; // passed through for non-memory instructions
    bool nop;
};

// global program and pipeline state
extern vector<Instructions> program;
extern int PC;
extern IFID ifid;
extern IDEX idex;
extern EXMEM exmem;
extern MEMWB memwb;

// init all pipeline registers to NOP
void initCPUState();

// advance pipeline by one clock cycle
void stepCycle(bool debug);

// returns true when all stages are empty and PC is past end
bool pipelineEmpty();

#endif