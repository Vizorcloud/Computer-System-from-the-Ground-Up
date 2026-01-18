/* File: larson.s
 * --------------
 * ***** TODO: add your file header comment here *****
 */

/*
 * The code below is the blink program from Lab 1.
 * Modify the code to implement the larson scanner for Assignment 1.
 * Be sure to use GPIO pins PB0-PB3 (or PB0-PB7) for your LEDs.
 */

    lui     a0, 0x2000       		         # a0 holds base addr PB group = 0x2000000
    li	    a1, 0x11111111  			 # a1 = value that sets all PB0-PB7 to output
    sw      a1, 0x30(a0)    			 # config PB0-PB7 as output
    
    li      t0, 0x80        			 # Initialize starting LED (PB7)
    li      t1, 0           			 # Initialize starting direction (right)

loop:
    sw      t0, 0x40(a0)     			 # writes current LED state for PB0-PB7

    lui     a2, 11000        			 # a2 = init countdown value

delay:
    addi    a2, a2, -1        			 # decrement a2	
    bne     a2, zero, delay   			 # keep counting down until a2 is zero

    li      t2, 0x01                		 # Store the value where PB0 is on
    beq     t2, t0, reverse_direction            # Reverse direction if PB0 is the current LED lit
    li      t2, 0x80                             # Store the value where PB7 is on
    beq     t2, t0, reverse_direction            # Reverse direction if PB7 is the current LED lit     

shift_led:
    beq     t1, zero, shift_right 		 # Shift current LED to the right if current direction is right
    slli    t0, t0, 1				 # Else shift current LED to the left by 1  
    j	    loop

shift_right:
    srli    t0, t0, 1 				 # Shifts current LED to the right by 1
    j       loop

reverse_direction:
    xori    t1, t1, 1      			 # Flip the current direction
    j       loop
