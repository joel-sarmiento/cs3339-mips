# MIPS Simulator

This simulator builds and runs a 5-stage pipelined MIPS processor that executes assembly programs written in the MIPS instruction set.
Below are the detailed instructions for **building** and **running** the simulator.

## Building the Simulator

### **Build Command**

```
g++ main.cpp cpu.cpp parser.cpp Memory.cpp Instructions.cpp -o mips_sim
```

This will produce an executable named `mips_sim`

## Running the Simulator
```
./mips_sim prog.asm
```
where `prog.asm ` is the input file with MIPS-like instructions.

## Using Debug Mode
Use the `-d` flag to show the additional pipeline state information each cycle
```
./mips_sim prog.asm -d
```
This prints the cycle number, PC value, instruction movement through stages, branch decisions, and ALU and memory operations.
