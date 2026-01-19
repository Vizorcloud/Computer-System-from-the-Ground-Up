/* File: extension.s
 * --------------
 * Purpose: EXTENSION - Program that lights 8 LEDs controlled by 8 distinct GPIO PB pins with Larson Scanner
 * functionality. Freeze current state functionality while pressing button and brightness gradient flow of nearby LEDs.
 * Name: Maxsem Garcia
 * Course: CS107E Tuesday Lab
 * Date Last Modified: Jan 18 2025
 */

/*
 * The code below is the blink program from Lab 1.
 * Modify the code to implement the larson scanner for Assignment 1.
 * Be sure to use GPIO pins PB0-PB3 (or PB0-PB7) for your LEDs.
 */

    lui     a0, 0x2000                           # a0 holds base addr PB group = 0x2000000
    li      a1, 0x11111111                       # a1 = value that sets all PB0-PB7 to output
    sw      a1, 0x30(a0)                         # config PB0-PB7 as output
    sw      zero, 0x60(a0)                        # config PC0 as input

    li      t0, 0x80                             # Initialize starting LED (PB7)
    li      t1, 1                                # Initialize starting direction (right)
    li      a4, 0                                # Initialize phase counter | Phase 0 - 100%-25% brightness LEDs ON

loop:
    lw      a3, 0x70(a0)
    andi    a3, a3, 1
    beq     a3, zero, freeze
    
    li      t3, 0                                # Create blank LED pattern

    or      t3, t0, t3				 # Turn on current LED
    
    slli    t4, t0, 1
    or      t3, t3, t4
    slli    t4, t0, 2
    or      t3, t3, t4
    slli    t4, t0, 3
    or      t3, t3, t4

    srli    t4, t0, 1
    or      t3, t3, t4
    srli    t4, t0, 2
    or      t3, t3, t4
    srli    t4, t0, 3
    or      t3, t3, t4

    andi    t3, t3, 0xFF	

    sw      t3, 0x40(a0)                         # Writes current LED state for PB0-PB7
   
    lui     a2, 5000                             # a2 = init countdown value

delay:
    addi    a2, a2, -1                           # Decrement a2 
    bne     a2, zero, delay                      # Keep counting down until a2 is zero

    beq     t1, zero, shift_left                 # Shift current LED to the left if current direction is left
    srli    t0, t0, 1                            # Else shift current LED to the right by 1
    li      t2, 0x01                             # Store the value where PB0 is toggled
    beq     t2, t0, reverse_direction            # Reverse direction if PB0 is the current LED lit
    j       loop

shift_left:
    slli    t0, t0, 1                            # Shifts current LED to the right by 1
    li      t2, 0x80                             # Store the value where PB7 is on
    beq     t2, t0, reverse_direction            # Reverse direction if PB7 is the current LED lit
    j       loop

reverse_direction:
    xori    t1, t1, 1                            # Flip the current direction
    j       loop

freeze:
    lw      a3, 0x70(a0)
    andi    a3, a3, 1
    bne     a3, zero, loop
    j       freeze
