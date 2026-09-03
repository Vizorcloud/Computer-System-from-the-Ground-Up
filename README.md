# Computer-System-from-the-Ground-Up
Built a complete bare-metal computer system on RISC-V from scratch — no OS, no libraries. Wrote GPIO/UART drivers, a heap allocator, a PS/2 keyboard driver (later reworked to use interrupts), a graphics library, and a shell, integrating them into a standalone, interactive command-line computer running entirely on my own code.

Project: Bare-Metal Computer System on RISC-V (Stanford CS107e)

The objective of this project was to build a complete, standalone computer system from the ground up on a RISC-V single-board computer (Mango Pi), with no operating system, no standard library, and no external dependencies — every layer of the stack, from raw hardware control to a usable interactive interface, written and debugged by hand.

Starting from bare RISC-V assembly and memory-mapped GPIO control, the system was built incrementally, layer by layer:

**Foundations**: Wrote the boot sequence, GPIO drivers, and a UART serial communication driver, along with a from-scratch C string library and printf implementation — establishing the basic ability to control hardware and observe program behavior with no debugging tools beyond what I built myself./n
**Memory** **management**: Implemented a stack backtrace tool using inline RISC-V assembly to walk frame pointers and resolve symbols from the ELF symbol table, plus a heap allocator (malloc/free) with block headers, free-list recycling, splitting, and coalescing — turning a naive bump allocator into a functioning dynamic memory system, validated under randomized stress testing.
**Input**: Built a PS/2 keyboard driver from the physical protocol up — decoding 11-bit scancode packets with parity and timeout error recovery — then a layered keyboard module handling modifiers, key events, and typed characters, and a command-line shell with a custom parser and command dispatch table.
**Output**: Implemented a framebuffer and graphics library supporting double-buffering, pixel/rectangle drawing with clipping, and text rendering, then built a text console on top of it handling cursor movement, line wrapping, and scrolling — giving the system a real graphical display.
**Concurrency**: Reworked the keyboard driver to be fully interrupt-driven instead of polling — configuring GPIO edge-triggered interrupts, writing an interrupt handler that reconstructs scancodes bit-by-bit across independent invocations, and using a ring buffer to safely hand data from interrupt context to the main program — eliminating dropped keystrokes under load.

The end result is a self-contained personal computer running entirely on code I wrote myself: a keyboard-driven, graphical command-line shell running on bare RISC-V hardware, packaged into a reusable library (libmymango) — a full vertical slice of computer systems, from transistors-adjacent hardware control up to a usable user interface.
