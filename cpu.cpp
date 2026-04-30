#include "cpu.h"
#include "Instructions.h"
#include "Memory.h"
#include <iostream>

using namespace std;

// global pipeline state registers and program
vector<Instructions> program;
int PC = 0;
IFID ifid;
IDEX idex;
EXMEM exmem;
MEMWB memwb;

// initialize all pipeline registers to NOP and reset PC
void initCPUState() {
    PC = 0;

    ifid.instr = makeNOP();
    ifid.pc = 0;
    ifid.nop = true;

    idex.instr = makeNOP();
    idex.pc = 0;
    idex.rsVal = 0;
    idex.rtVal = 0;
    idex.imm = 0;
    idex.nop = true;

    exmem.instr = makeNOP();
    exmem.aluResult = 0;
    exmem.rtVal = 0;
    exmem.takeBranch = false;
    exmem.branchTarget = 0;
    exmem.nop = true;

    memwb.instr = makeNOP();
    memwb.memData = 0;
    memwb.aluResult = 0;
    memwb.nop = true;
}

static bool isNOP(const Instructions& ins) {
    return ins.op == NOP_OP;
}

bool pipelineEmpty() {
    bool allNops = isNOP(ifid.instr) && isNOP(idex.instr) && isNOP(exmem.instr) && isNOP(memwb.instr);
    bool pcPastEnd = (PC >= (int)program.size());
    return allNops && pcPastEnd;
}

void stepCycle(bool debug) {
    // copy current pipeline registers into next cycle registers
    IFID next_ifid = ifid;
    IDEX next_idex = idex;
    EXMEM next_exmem = exmem;
    MEMWB next_memwb = memwb;

    // writeback: write ALU result or memory data back to register file
    if(!memwb.nop && !isNOP(memwb.instr)) {
        Instructions ins = memwb.instr;
        switch(ins.op) {
            // r type instructions write to rd
            case ADD: case SUB: case MUL: case AND_OP: case OR_OP: case SLL: case SRL:
                if(ins.rd != 0) {
                    R[ins.rd] = memwb.aluResult;
                }
                break;
            // ADDI writes to rt
            case ADDI:
                if(ins.rt != 0) {
                    R[ins.rt] = memwb.aluResult;
                }
                break;
            // LW writes loaded memory data to rt
            case LW:
                if(ins.rt != 0) {
                    R[ins.rt] = memwb.memData;
                }
                break;
            default:
                break;
        }
    }

    // memory: perform load/store operations
    if(!exmem.nop && !isNOP(exmem.instr)) {
        Instructions ins = exmem.instr;
        next_memwb.instr = ins;
        next_memwb.nop = false;
        next_memwb.aluResult = exmem.aluResult;
        next_memwb.memData = 0;

        switch (ins.op) {
            // load word from memory at ALU computed address
            case LW: {
                int address = exmem.aluResult;
                if(address >= 0 && address < MEM_SIZE) {
                    next_memwb.memData = MEM[address];
                }
                break;     
            }
            // store word to memory at ALU-computed address
            case SW: {
                int address = exmem.aluResult;
                if (address >= 0 && address < MEM_SIZE) {
                    MEM[address] = exmem.rtVal;
                }
                break;
            }
            default:
                break;
        }
    } else {
        next_memwb.instr = makeNOP();
        next_memwb.nop = true;
    }

    // execution: ALU operations and data forwarding
    if(!idex.nop && !isNOP(idex.instr)) {
        Instructions ins = idex.instr;
        next_exmem.instr = ins;
        next_exmem.nop = false;
        next_exmem.rtVal = idex.rtVal;
        next_exmem.takeBranch = false;
        next_exmem.branchTarget = 0;

        int a = idex.rsVal;
        int b = idex.rtVal;
        int imm = idex.imm;
        int result = 0;

        // forwarding from ex/mem
        if(!exmem.nop && !isNOP(exmem.instr)) {
            Opcode exOp = exmem.instr.op;

            // r type: forward from rd
            if(exOp == ADD || exOp == SUB || exOp == MUL || exOp == AND_OP 
                || exOp == OR_OP || exOp == SLL || exOp == SRL) {
                int exDest = exmem.instr.rd;
                if(exDest != 0) {
                    if(idex.instr.rs == exDest) {
                        a = exmem.aluResult;
                    }
                    if(idex.instr.rt == exDest) {
                        b = exmem.aluResult;
                    }
                }
            }
            // ADDI/LW: forward from rt
            if(exOp == ADDI || exOp == LW) {
                int exDest = exmem.instr.rt;
                if(exDest != 0) {
                    if(idex.instr.rs == exDest) {
                        a = exmem.aluResult;
                    }
                    if(idex.instr.rt == exDest) {
                        b = exmem.aluResult;
                    }
                }
            }
        }

        // forward from mem/wb
        if(!memwb.nop && !isNOP(memwb.instr)) {
            Opcode wbOp = memwb.instr.op;
            int wbValue = memwb.aluResult;

            // LW forwards from memData
            if(wbOp == LW) {
                wbValue = memwb.memData;
            }

            // r type: forward from rd
            if(wbOp == ADD || wbOp == SUB || wbOp == MUL || wbOp == AND_OP 
                || wbOp == OR_OP || wbOp == SLL || wbOp == SRL) {
                int wbDest = memwb.instr.rd;
                if(wbDest != 0) {
                    if(idex.instr.rs == wbDest && a == idex.rsVal) {
                        a = wbValue;
                    }
                    if(idex.instr.rt == wbDest && b == idex.rtVal) {
                        b = wbValue;
                    }
                }
            }

            // ADDI/LW: forward from rt
            if(wbOp == ADDI || wbOp == LW) {
                int wbDest = memwb.instr.rt;
                if(wbDest != 0) {
                    if(idex.instr.rs == wbDest && a == idex.rsVal) {
                        a = wbValue;
                    }
                    if(idex.instr.rt == wbDest && b == idex.rtVal) {
                        b = wbValue;
                    }
                }
            }
        }

        // ALU operation
        switch(ins.op) {
            case ADD: 
                result = a + b; 
                break;
            case ADDI: 
                result = a + imm; 
                break;
            case SUB: 
                result = a - b; 
                break;
            case MUL: 
                result = a * b; 
                break;
            case AND_OP: 
                result = a & b;
                break;
            case OR_OP: 
                result = a | b; 
                break;
            case SLL: 
                result = b << imm; 
                break;
            case SRL: 
                result = (unsigned int)b >> imm; 
                break;
            case LW:
            case SW:
                result = a + imm;
                break;
            default:
                break;
        }

        next_exmem.aluResult = result;
     } else {
        next_exmem.instr = makeNOP();
        next_exmem.nop = true;
        next_exmem.takeBranch = false;
        next_exmem.branchTarget = 0;
     }

     // instruction decode: read register values and handle branches/jumps early
    if(!ifid.nop && !isNOP(ifid.instr)) {
        Instructions ins = ifid.instr;

        next_idex.instr = ins;
        next_idex.nop = false;
        next_idex.pc = ifid.pc;
        next_idex.imm = ins.imm;

        // defaults to 0 if value read is out of bounds
        if (ins.rs >= 0 && ins.rs < NUM_REGS) {
            next_idex.rsVal = R[ins.rs];
        } else {
            next_idex.rsVal = 0;
        }

        if (ins.rt >= 0 && ins.rt < NUM_REGS) {
            next_idex.rtVal = R[ins.rt];
        } else {
            next_idex.rtVal = 0;
        }
        
        // BEQ: resolve branch in id stage and flush if taken
        if(ins.op == BEQ) {
            int a = next_idex.rsVal;
            int b = next_idex.rtVal;
            if(a == b) {
                PC = ins.address;
                
                // flush instructions that enter pipeline after branch
                next_ifid.instr = makeNOP();
                next_ifid.nop = true;

                next_idex.instr = makeNOP();
                next_idex.nop = true;
                next_idex.rsVal = 0;
                next_idex.rtVal = 0;
                next_idex.imm = 0;

                next_exmem.instr = makeNOP();
                next_exmem.nop = true;
            }
        // J: unconditional jump so always flush
        } else if (ins.op == J) {
            PC = ins.address;

            next_ifid.instr = makeNOP();
            next_ifid.nop = true;

            next_idex.instr = makeNOP();
            next_idex.nop = true;
            next_idex.rsVal = 0;
            next_idex.rtVal = 0;
            next_idex.imm = 0;

            next_exmem.instr = makeNOP();
            next_exmem.nop = true;
        }
    } else {
        next_idex.instr = makeNOP();
        next_idex.nop = true;
        next_idex.rsVal = 0;
        next_idex.rtVal = 0;
        next_idex.imm = 0;
    }

    // instruction fetch: fetch next instruction from program using PC
        if(PC < (int)program.size()) {
            next_ifid.instr = program[PC];
            next_ifid.pc = PC;
            next_ifid.nop = false;
            PC++;
        } else {
            next_ifid.instr = makeNOP();
            next_ifid.pc = PC;
            next_ifid.nop = true;
        }

    // next pipeline state
    ifid = next_ifid;
    idex = next_idex;
    exmem = next_exmem;
    memwb = next_memwb;

}