/* File: extension.s
 * --------------
 * Purpose: EXTENSION - Program that lights 8 LEDs controlled by 8 distinct GPIO PB pins with Larson Scanner
 * functionality. Freeze current state functionality while pressing button and brightness gradient flow of nearby LEDs.
 * Name: Maxsem Garcia
 * Course: CS107E Tuesday Lab
 * Date Last Modified: Jan 19 2025
 */

/*
 * The code below is the blink program from Lab 1.
 * Modify the code to implement the larson scanner for Assignment 1.
 * Be sure to use GPIO pins PB0-PB3 (or PB0-PB7) for your LEDs.
 */

    lui     a0, 0x2000                           # a0 holds base addr PB group = 0x2000000
    li      a1, 0x11111111                       # a1 = value that sets all PB0-PB7 to output
    sw      a1, 0x30(a0)                         # config PB0-PB7 as output
    sw      zero, 0x60(a0)                       # config PC0 as input

    li      t0, 0x80                             # Initialize starting LED (PB7)
    li      t1, 1                                # Initialize starting direction (right)
    li      a4, 0                                # Initialize phase counter | Phase 0 - 100%-25% brightness LEDs ON

loop:
    lui     a2, 60                              # a2 = init countdown value

delay:
    li      t3, 0     				 # Create Base LED pattern                           

    or      t3, t0, t3   	  		 # Light up current LED

    li      t5, 3				 # t5 = Value that represents Phase 3
    bge     a4, t5, skip_75                      # Skip lighting the closest neighbors (75% brightness) if were past phase 2
    slli    t4, t0, 1 				 # Store the value that turns on the 1st neighbor LED to the left
    or      t3, t3, t4				 # Mask this value onto the LED pattern
    srli    t4, t0, 1   	 		 # Store the value that turns on the 1st neighbor LED to the right
    or      t3, t3, t4    			 # Mask this value onto the LED pattern

skip_75:
    li      t5, 2				 # t5 = Value that represents Phase 2
    bge     a4, t5, skip_50			 # Skip lighting the 2nd closest neighbors (50% brightness) if were past phase 1
    slli    t4, t0, 2				 # Store the value that turns on the 2nd neighbor LED to the left
    or      t3, t3, t4				 # Mask this value onto the LED pattern
    srli    t4, t0, 2				 # Store the value that turns on the 2nd neighbor LED to the right
    or      t3, t3, t4				 # Mask this value onto the LED pattern

skip_50:
    li      t5, 1				 # t5 = Value that represents Phase 3
    bge     a4, t5, skip_25			 # Skip lighting the 3rd closest neighbors (25% brightness) if were past phase 0
    slli    t4, t0, 3				 # Store the value that turns on the 3rd neighbor LED to the left
    or      t3, t3, t4				 # Mask this value onto the LED pattern
    srli    t4, t0, 3				 # Store the value that turns on the 3rd neighbor LED to the right
    or      t3, t3, t4				 # Mask this value onto the LED pattern
   
skip_25:
    andi    t3, t3, 0xFF			 # Masking to extract the least significant byte and prevent shift overflow damage
    sw      t3, 0x40(a0)    			 # Write the LED pattern into PB Data register to light LEDS

    addi    a4, a4, 1				 # Increment Phase counter
    andi    a4, a4, 3				 # This resets the counter to 0 if it goes above 3

    lw      a3, 0x70(a0)  			 # Read the data from the button
    andi    a3, a3, 1				 # Mask to keep the least significant bit
    beq     a3, zero, loop                       # If this bit is 0 (Pressed) then freeze the state of the LED pattern and brightnesses

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
