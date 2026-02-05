Include information for grader about your assign3 here

My completed submission attempts the full 4 credit extension:

Supported instruction types:

    R-type – standard register-register instructions (add, sub, and, or, etc.)
    I-type – immediate arithmetic and shift instructions (addi, ori, slli, srai, etc.)
    SB-type – conditional branch instructions (beq, bne, etc.), with correct target address calculation.
    UJ-type – jump instructions (jal), with absolute target address calculation.

When printing with %pI, the output format is: 0x<address>: <mnemonic> <operands>
i.e "0x000000004fffff7c: jal zero,0x4fffff7c"

- Immediate values are sign-extended where appropriate.
- Absolute addresses are printed instead of relative offsets to make branch and jump targets easier to read.
- Treats each instruction as a 32-bit word, using only the lower 32 bits for decoding, which ensures correct interpretation even in 64-bit memory layouts.
- Added padding between 0x<address>: and <mnemonic> to match closer to the gdb disassembler

If an instruction is unrecognized, it prints as: .word 0x<encoded instruction>
