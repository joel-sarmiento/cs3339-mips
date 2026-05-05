#include <iostream>
#include "Memory.h"
#include "cpu.h"
#include "parser.h"

using namespace std;

int main(int argc, char *argv[])
{
    string filename = "prog.asm";
    bool debug = false;

    if (argc > 1)
    {
        filename = argv[1];
    }

    if (argc > 2 && string(argv[2]) == "-d")
    {
        debug = true;
    }

    if (!loadProgram(filename, program))
    {
        cerr << "Failed to load program " << filename << "\n";
        return 1;
    }

    initRegisters();
    initMemory();
    initCPUState();

    int cycle = 0;

    while (!pipelineEmpty())
    {
        if (debug)
        {
            cout << "PC: " << PC << "\n";
            cout << "IFID: op=" << ifid.instr.op << " pc=" << ifid.pc << " nop=" << ifid.nop << "\n";
            cout << "IDEX: op=" << idex.instr.op << " rsVal=" << idex.rsVal << " rtVal=" << idex.rtVal << " imm=" << idex.imm << " nop=" << idex.nop << "\n";
            cout << "EXMEM: op=" << exmem.instr.op << " aluResult=" << exmem.aluResult << " takeBranch=" << exmem.takeBranch << " target=" << exmem.branchTarget << " nop=" << exmem.nop << "\n";
            cout << "MEMWB: op=" << memwb.instr.op << " aluResult=" << memwb.aluResult << " memData=" << memwb.memData << " nop=" << memwb.nop << "\n";

            Opcode op = idex.instr.op;
            bool RegWrite = (op == ADD || op == ADDI || op == SUB || op == MUL || op == AND_OP || op == OR_OP || op == SLL || op == SRL || op == LW);
            bool MemRead = (op == LW);
            bool MemWrite = (op == SW);
            bool Branch = (op == BEQ);
            bool Jump = (op == J);

            cout << "Control: RegWrite=" << RegWrite << " MemRead=" << MemRead << " MemWrite=" << MemWrite << " Branch=" << Branch << " Jump=" << Jump << "\n\n";

            // added registers display for each cycle
            cout << "Registers:\n";
            for (int i = 0; i < NUM_REGS; i++)
            {
                cout << "R[" << i << "] = " << R[i] << "\n";
            }
            cout << "\n";

            // added first 32 words of memory
            cout << "Memory (first 32 words):\n";
            for (int i = 0; i < 32; ++i)
            {
                cout << "MEM[" << i << "] = " << MEM[i] << "\n";
            }
            cout << "\n";
        }
        stepCycle(debug);
        cycle++;
    }
    cout << "Execution finished in " << cycle << " cycles.\n\n";
    cout << "Final Registers:\n";
    for (int i = 0; i < NUM_REGS; ++i)
    {
        cout << "R[" << i << "] = " << R[i] << "\n";
    }
    cout << "\n";
    cout << "Final Memory (first 32 words):\n";
    for (int i = 0; i < 32; ++i)
    {
        cout << "MEM[" << i << "] = " << MEM[i] << "\n";
    }
    return 0;
}
