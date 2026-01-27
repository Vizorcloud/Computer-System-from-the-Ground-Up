# File: timer_asm.s
# ------------------
# TODO: add your file header comment here

.attribute arch, "rv64imac_zicsr"

.globl timer_get_ticks
timer_get_ticks:
    csrr a0, time
    ret
