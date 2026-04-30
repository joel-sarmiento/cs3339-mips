# Initialize some registers with ADDI
ADDI $1, $0, 5        # R1 = 5
ADDI $2, $0, 3        # R2 = 3

# Arithmetic and logic (R-type)
ADD  $3, $1, $2       # R3 = 5 + 3 = 8
SUB  $4, $1, $2       # R4 = 5 - 3 = 2
MUL  $5, $3, $4       # R5 = 8 * 2 = 16
AND  $6, $1, $2       # R6 = 5 & 3 = 1
OR   $7, $1, $2       # R7 = 5 | 3 = 7

# Shifts
SLL  $8, $1, 2        # R8 = 5 << 2 = 20
SRL  $9, $3, 1        # R9 = 8 >> 1 = 4

# Memory operations
SW   $5, 0($0)        # MEM[0] = 16
LW   $10, 0($0)       # R10 = MEM[0] = 16

# Branch taken and NOP in the delay slot region
BEQ  $10, $5, skip    # if R10 == R5 then jump to 'skip'
NOP                   # will be flushed/ignored when branch is taken

# This line should be skipped if BEQ is taken
ADDI $11, $0, 999     # should NOT execute

skip:
# Jump example (J) 
J    end              # unconditional jump over next ADDI
ADDI $12, $0, 123     # should NOT execute because of jump

end:
NOP                   # final NOP, safe endpoint
