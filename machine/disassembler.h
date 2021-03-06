/*
  Disassembler for 8080 processor:

  Registers : A, B, C, D, E, H, L
  Program Counter (PC)
  Stack Pointer (SP)
  
  Operations Info:
  - Operations sometimes work on register pairs
       - BC is a pair, DE, HL
  - A = special register
  - HL = special, used as address for data reads or writes to memory
  - RST = executes code at a fixed location, sometimes interrupt handlers
       - Interrupts can either be generated via RST, or sources external to
       8080, like the code at beginning of the ROM (which have interrupt service
       routines [ISRs]), note that ROM = read only memory
  - Not all instructions are only one byte
 
 */


// codebuffer = 8080 assembly code string
// pc = program counter, or offset into code string
int Disassemble8080Code (unsigned char *codebuffer, int pc);

// read in from codebuffer and debug code snippet, size is size of snippet
void decode8080Code (unsigned char *codebuffer, int size);

// read in from file of title filename and debug the 8080 program
// into assembly code
void decode8080CodeFile (char *fileName);
