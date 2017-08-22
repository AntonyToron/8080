/*
  File name : CPU.c
  
  Description : contains the bulk of the information for the 8080 cpu
  Author : Antony Toron

 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "CPU.h"

uint8_t flag = 0;
void startPrintingOut() {
  flag = 1;
}


struct ConditionCodes { // specifying bit count (1 bit flags/flip flops)
  uint8_t z : 1;   // zero flag
  uint8_t s : 1;   // sign flag (0 = POSITIVE, 1 = NEGATIVE)
  uint8_t p : 1;   // parity flag (0 = ODD, 1 = EVEN)
  uint8_t c : 1;   // carry flag (0 = no carry, 1 = carry)
  uint8_t ac : 1;  // auxiliary carry (carry out of 4 bit accumulator - DAA) 
  uint8_t pad : 3; // padding (8 bits)
  // IE: flip flop for interrupt enabled
};

// PORT INFORMATION FOR 8080

/* Ports:     */
/*     Read 1     */
/*     BIT 0   coin (0 when active)     */
/*         1   P2 start button     */
/*         2   P1 start button     */
/*         3   ?     */
/*         4   P1 shoot button     */
/*         5   P1 joystick left     */
/*         6   P1 joystick right     */
/*         7   ? */

/*     Read 2     */
/*     BIT 0,1 dipswitch number of lives (0:3,1:4,2:5,3:6)     */
/*         2   tilt 'button'     */
/*         3   dipswitch bonus life at 1:1000,0:1500     */
/*         4   P2 shoot button     */
/*         5   P2 joystick left     */
/*         6   P2 joystick right     */
/*         7   dipswitch coin info 1:off,0:on     */

/*     Read 3      shift register result     */

/*     Write 2     shift register result offset (bits 0,1,2)     */
/*     Write 3     sound related     */
/*     Write 4     fill shift register     */
/*     Write 5     sound related     */
/*     Write 6     strange 'debug' port? eg. it writes to this port when     */
/*             it writes text to the screen (0=a,1=b,2=c, etc)     */

/*     (write ports 3,5,6 can be left unemulated, read port 1=$01 and 2=$00     */
/*     will make the game run, but but only in attract mode)  */ 

// describe the behavior for each respective port
// in should return a value to read into accumulator
// out takes value to put into output
// SHOULD BE IMPLEMENTED FOR I/O TO WORK
struct Drivers {
  uint8_t (*in[8]) (); // 8 IN ports on machine
  void (*out[8]) (uint8_t ac);   // 8 OUT ports on machine
};

// Quick registers notes:
// ADD B = 8 bit operation, adding contents of register B to accumulator A
// PUSH B = 16 bit operation, pushing B and C onto the stack
// ^ demonstration of the use of "16 bit" registers vs. regular use

// address = stored in HL register pair for memory instructions
// 8 bit registers, A, B, C, D, E, H, and L
// 8 bit accumulator referred to by A, and
// the rest of the registers are general purpose
      
// "16 bit" registers - BC = B, DE = D, HL = H


struct State8080 {
  uint8_t registers[8];// registers B, C, D, E, H, L, Memory, A
  uint16_t sp;         // stack pointer
  uint16_t pc;         // program counter
  uint8_t *memory;     // RAM, pointer to sequence of unsigned char/uint8
  ConditionCodes_T cc;
  Drivers_T drivers;
  uint8_t IE : 1;          // interrupt enabled flip flop
  uint8_t *interrupts; // interrupt queue
};


static void UnimplementedInstruction(State8080_T state) {
  // PC will have been incremented, so it should be decremented back
  unsigned char *opcode = &state->memory[state->pc];
  
  fprintf (stderr, "Unimplemented instruction : $%02x\n", *opcode);
  exit(1);
}

static uint8_t Parity(uint16_t target) {
  if (target % 2 == 0)
    return 1;
  return 0;
}

static void UpdateCCSimple(State8080_T state, uint16_t result) {
  state->cc->z = ((result & 0xff) == 0); // check if 0, AND with all 1s
  state->cc->s = ((result & 0x80) != 0); // check if bit 7 is set
  state->cc->c = (result > 0xff);        // check if sum > 256
  state->cc->p = Parity(result & 0xff);  // check parity
}

static void UpdateCCAll(State8080_T state, uint16_t result, uint8_t lsb) {
  state->cc->z = ((result & 0xff) == 0); // check if 0, AND with all 1s
  state->cc->s = ((result & 0x80) != 0); // check if bit 7 is set
  state->cc->c = (result > 0xff);        // check if sum > 256
  state->cc->p = Parity(result & 0xff);  // check parity
  state->cc->ac = (lsb > 9);             // set ac if 4 bit carry
}

static void UpdateCCSub(State8080_T state, uint16_t result, uint8_t lsb) {
  state->cc->z = ((result & 0xff) == 0); // check if 0, AND with all 1s
  state->cc->s = ((result & 0x80) != 0); // check if bit 7 is set
  state->cc->c = (result < 0xff);        // check if sum > 256
  state->cc->p = Parity(result & 0xff);  // check parity
  state->cc->ac = (lsb > 9);             // set ac if 4 bit carry
}

static void UpdateCCZeroSignParity(State8080_T state, uint8_t result, uint8_t lsb) {
  state->cc->z = ((result & 0xff) == 0);
  state->cc->s = ((result & 0x80) != 0);
  state->cc->p = Parity(result & 0xff);
  state->cc->ac = (lsb > 9);
}

static void PushPC(State8080_T state) {
  state->memory[state->sp - 1] = (uint8_t) ((state->pc >> 8) & 0xff);
  state->memory[state->sp - 2] = (uint8_t) (state->pc & 0x00ff);

  state->sp -= 2;
}

static void WRITE_MEM(State8080_T state, uint16_t address, uint8_t byte) {
  state->memory[address] = byte;
}

static void WRITE_MEM_HL(State8080_T state, uint8_t byte) {
  uint16_t offset = (state->registers[4]<<8) | (state->registers[5]);

  state->memory[offset] = byte;
}

static uint8_t READ_MEM_HL(State8080_T state) {
  uint16_t offset = (state->registers[4]<<8) | (state->registers[5]);

  return state->memory[offset];
}

static uint8_t GetRegister(State8080_T state, uint8_t reg) {
  switch (reg) {
  case 0x06:
    {
      // H = high order bits, L = lower order bits, so shift H 8 bits up
      uint16_t offset = (state->registers[4]<<8) | (state->registers[5]);
      return state->memory[offset];
      break;
    }
  default:
    return state->registers[reg];
  }
}

static void MOV (State8080_T state) {
  unsigned char *opcode = &state->memory[state->pc];
  uint8_t dest = ((*opcode) & 0x38)>>3;
  uint8_t source = (*opcode) & 0x07;

  if (dest == 6 && source != 6) {
    WRITE_MEM_HL(state, state->registers[source]);
  }
  else if (dest != 6 && source == 6) {
    state->registers[dest] = READ_MEM_HL(state);
  }
  else {
    state->registers[dest] = state->registers[source];
  }
}

static void MVI (State8080_T state) {
  unsigned char *opcode = &state->memory[state->pc];
  uint8_t dest = ((*opcode) & 0x38)>>3;

  if (dest == 6) {
    WRITE_MEM_HL(state, opcode[1]);
  }
  else {
    state->registers[dest] = opcode[1];
  }
}

// ADD register r from opcode to accumulator (a)
static void ADD_R (State8080_T state) {
  unsigned char *opcode = &state->memory[state->pc];
  uint8_t reg = (*opcode) & 0x07;
  uint8_t reg_value = GetRegister(state, reg);
  uint16_t sum = (uint16_t) state->registers[7] + (uint16_t) reg_value;
  uint8_t lsb = (state->registers[7] & 0x0f) + (reg_value & 0x0f);
  
  UpdateCCAll(state, sum, lsb);
  state->registers[7] = sum & 0xff;              // update accumulator
}

// ADD register r from opcode to accumulator (a) with carry
static void ADC_R (State8080_T state) {
  unsigned char *opcode = &state->memory[state->pc];
  uint8_t reg = (*opcode) & 0x07;
  uint8_t reg_value = GetRegister(state, reg);
  uint16_t sum = (uint16_t) state->registers[7] + (uint16_t) reg_value + (uint16_t) state->cc->c;
  uint8_t lsb = (state->registers[7] & 0x0f) + (reg_value & 0x0f) + state->cc->c;
  
  UpdateCCAll(state, sum, lsb);
  state->registers[7] = sum & 0xff;              // update accumulator
}

// ADD register r from opcode to accumulator (a)
static void SUB_R (State8080_T state) {
  unsigned char *opcode = &state->memory[state->pc];
  uint8_t reg = (*opcode) & 0x07;
  uint8_t reg_value = GetRegister(state, reg);
  uint16_t result = (uint16_t) state->registers[7] - reg_value;
  uint8_t lsb = (state->registers[7] & 0x0f) - (reg_value & 0x0f);
  
  UpdateCCSub(state, result, lsb);
  state->registers[7] = result & 0xff;              // update accumulator
}

// SUB register r from accumulator (a) with carry
static void SBB_R (State8080_T state) {
  unsigned char *opcode = &state->memory[state->pc];
  uint8_t reg = (*opcode) & 0x07;
  uint8_t reg_value = GetRegister(state, reg);
  uint16_t result = (uint16_t) state->registers[7] - (uint16_t) reg_value - (uint16_t) state->cc->c;
  uint8_t lsb = (state->registers[7] & 0x0f) - ((reg_value + state->cc->c) & 0x0f);
  
  UpdateCCSub(state, result, lsb); 
  state->registers[7] = result & 0xff;              // update accumulator
}

// ANA register r, r & a -> a, set sign, zero, parity. carry = 0
static void ANA (State8080_T state) {
  uint8_t result;
  unsigned char *opcode = &state->memory[state->pc];
  uint8_t reg = (*opcode) & 0x07;

  if (reg == 6) {
    result = state->registers[7] & READ_MEM_HL(state);
    state->registers[7] = result;
  }
  else {
    result = state->registers[7] & state->registers[reg];
    state->registers[7] = result;
  }

  UpdateCCSimple(state, result);
  state->cc->c = 0; // set carry to 0 just in case
}

// XRA register r, r ^ a -> a, set sign, zero, parity. carry = 0
static void XRA (State8080_T state) {
  uint8_t result;
  unsigned char *opcode = &state->memory[state->pc];
  uint8_t reg = (*opcode) & 0x07;

  if (reg == 6) {
    result = state->registers[7] ^ READ_MEM_HL(state);
    state->registers[7] = result;
  }
  else {
    result = state->registers[7] ^ state->registers[reg];
    state->registers[7] = result;
  }

  UpdateCCSimple(state, result);
  state->cc->c = 0; // set carry to 0 just in case
  state->cc->ac = 0; // ac is zeroed
}

// OR register r, r | a -> a, same cc as XRA
static void ORA (State8080_T state) {
  uint8_t result;
  unsigned char *opcode = &state->memory[state->pc];
  uint8_t reg = (*opcode) & 0x07;

  if (reg == 6) {
    result = state->registers[7] | READ_MEM_HL(state);
    state->registers[7] = result;
  }
  else {
    result = state->registers[7] | state->registers[reg];
    state->registers[7] = result;
  }

  UpdateCCSimple(state, result);
  state->cc->c = 0; // set carry to 0 just in case
  state->cc->ac = 0; // ac is zeroed
  
}

static void CMP (State8080_T state) {
  unsigned char *opcode = &state->memory[state->pc];
  uint8_t reg = ((*opcode) & 0x07);
  uint8_t reg_value = GetRegister(state, reg);
  uint16_t result = (state->registers[7] - reg_value);
  uint8_t lsb = (state->registers[7] & 0x0f) - (reg_value & 0x0f);
  
  UpdateCCAll(state, result, lsb);
  if (state->registers[7] >= reg_value)
    state->cc->c = 0;
  else if (state->registers[7] < reg_value)
    state->cc->c = 1;
}

// INR register r from opcode
static void INR_R (State8080_T state) {
  unsigned char *opcode = &state->memory[state->pc];
  uint8_t reg = ((*opcode) & 0x38)>>3;

  // source for CC info: http://altairclone.com/downloads/manuals/8080%20Programmers%20Manual.pdf vii

  switch (reg) {
  case 0x06:
    {
      // H = high order bits, L = lower order bits, so shift H 8 bits up
      uint16_t offset = (state->registers[4]<<8) | (state->registers[5]);
      uint8_t lsb = (state->memory[offset] & 0x0f) + 1;
      state->memory[offset] = state->memory[offset] + 1;
      UpdateCCZeroSignParity(state, state->memory[offset], lsb);
      break;
    }
  default:
    {
      uint8_t lsb = (state->registers[reg] & 0x0f) + 1;
      state->registers[reg] = state->registers[reg] + 1;
      UpdateCCZeroSignParity(state, state->registers[reg], lsb);
      break;
    }
  }
}

// DCR register r from opcode
static void DCR_R (State8080_T state) {
  unsigned char *opcode = &state->memory[state->pc];
  uint8_t reg = ((*opcode) & 0x38)>>3;

  switch (reg) {
  case 0x06:
    {
      // H = high order bits, L = lower order bits, so shift H 8 bits up
      uint16_t offset = (state->registers[4]<<8) | (state->registers[5]);
      uint8_t lsb = (state->memory[offset] & 0x0f) - 1;
      state->memory[offset] = state->memory[offset] - 1;
      UpdateCCZeroSignParity(state, state->memory[offset], lsb);
      break;
    }
  default:
    {
      uint8_t lsb = (state->registers[reg] & 0x0f) - 1;
      state->registers[reg] = state->registers[reg] - 1;
      UpdateCCZeroSignParity(state, state->registers[reg], lsb);
      break;
    }
  }
}
 
static void JMP_conditional (State8080_T state, uint8_t condition) {
   unsigned char *opcode = &state->memory[state->pc];
   if (condition) {
     uint16_t addr = (opcode[2] << 8) | (opcode[1]); // little-endian
     state->pc = addr;
   }
   else
     state->pc += 3;
}

static void RET_conditional (State8080_T state, uint8_t condition) {
  if (condition) {
    // PCL(low order bits) <- sp, PCH <- sp + 1, SP <- sp + 2
    uint16_t updatedPC = (state->memory[state->sp + 1] << 8) | (state->memory[state->sp]);
    
    state->pc = updatedPC;
    state->sp += 2;
  }
  else {
    state->pc += 1;
  }
}

static void CALL_conditional (State8080_T state, uint8_t condition) {
  unsigned char *opcode = &state->memory[state->pc];

  state->pc += 1; // for op
  if (condition) {
    // (sp - 1) <- PCH, (sp - 2) <- PCL, SP -= 2, PC <- addr
    state->pc += 2; // for data bytes
    
    PushPC(state);
    
    state->pc = (opcode[2] << 8) | (opcode[1]); // little-endian
  }
  else
    state->pc += 2; // for data bytes
}

// similar to call, but calls interrupt code
void RST(State8080_T state, uint8_t opcode) {  
  // push program counter
  PushPC(state);

  // 11AAA111 -> 00AAA000
  uint16_t addr = (opcode & 0x38) & 0xffff;
  state->pc = addr;
}
uint8_t times = 0;
void Emulate8080Op(State8080_T state, unsigned char *opcode) {
  #ifdef INSTRUCTION_DEBUGGING
  if (state->pc == 0x8)
    printf ("RST 1\n");
  else if (state->pc == 0x10)
    printf ("RST 2\n");
  else if (state->pc == 0x87)
    printf ("Leaving RST\n");
  #endif
  
  // NOTE: many of the instructions update PC <- PC + 1, for the op
  //if (state->pc == 0x0133) {
  //  printf ("Got to drawing");
  //}
  // inx sp, dcx sp, not implemented
  switch (*opcode) {
  case 0x00:               // NOP
    state->pc += 1;
    break;        
    // --------------- MOVE/LOADS/STORES ----------------------
    
    // TODO: FORMAT THESE COMMENTS FOR MOV PROPERLY
    // -------------------- MOV -------------------------------
  case 0x40:               // MOV B, B
  case 0x41:               // MOV B, C
  case 0x42:               // MOV B, D
  case 0x43:               // MOV B, E
  case 0x44:               // MOV B, H
  case 0x45:               // MOV B, L
  case 0x46:               // MOV B, Memory
  case 0x47:               // MOV B, Accumulator
  case 0x48:               // MOV C, B
  case 0x49:               // MOV C, C
  case 0x4A:               // MOV B, C
  case 0x4B:               // MOV B, D
  case 0x4C:               // MOV B, E
  case 0x4D:               // MOV B, C
  case 0x4E:               // MOV B, D
  case 0x4F:               // MOV B, E
  case 0x50:               // MOV B, 
  case 0x51:               // MOV B, C
  case 0x52:               // MOV B, D
  case 0x53:               // MOV B, E
  case 0x54:               // MOV B, H
  case 0x55:               // MOV B, L
  case 0x56:               // MOV B, Memory
  case 0x57:               // MOV B, Accumulator
  case 0x58:
  case 0x59:               // MOV B, E
  case 0x5A:               // MOV B, C
  case 0x5B:               // MOV B, D
  case 0x5C:               // MOV B, E
  case 0x5D:               // MOV B, C
  case 0x5E:               // MOV B, D
  case 0x5F:               // MOV B, E
  case 0x60:               // MOV B, 
  case 0x61:               // MOV B, C
  case 0x62:               // MOV B, D
  case 0x63:               // MOV B, E
  case 0x64:               // MOV B, H
  case 0x65:               // MOV B, L
  case 0x66:               // MOV B, Memory
  case 0x67:               // MOV B, Accumulator
  case 0x68:
  case 0x69:               // MOV B, E
  case 0x6A:               // MOV B, C
  case 0x6B:               // MOV B, D
  case 0x6C:               // MOV B, E
  case 0x6D:               // MOV B, C
  case 0x6E:               // MOV B, D
  case 0x6F:               // MOV B,
  case 0x70:               // MOV B, 
  case 0x71:               // MOV B, C
  case 0x72:               // MOV B, D
  case 0x73:               // MOV B, E
  case 0x74:               // MOV B, H
  case 0x75:               // MOV B, L
    // no 76 because memory -> memory
  case 0x77:               // MOV B, Accumulator
  case 0x78:
  case 0x79:               // MOV B, E
  case 0x7A:               // MOV B, C
  case 0x7B:               // MOV B, D
  case 0x7C:               // MOV B, E
  case 0x7D:               // MOV B, C
  case 0x7E:               // MOV B, D
  case 0x7F:               // MOV B, E
    MOV (state);
    state->pc += 1;
    // no cc affected
    break;

    // --------------------- MVI ------------------------------
  case 0x06:               // MVI B, move immediate to B
  case 0x0E:               // MVI C, move immediate to C
  case 0x16:               // MVI D, move immediate to D
  case 0x1E:               // MVI E, move immediate to E
  case 0x26:               // MVI H, move immediate to H
  case 0x2E:               // MVI L, move immediate to L
  case 0x36:               // MVI memory, move immediate to memory
  case 0x3E:               // MVI A, move immediate to accumulator
    MVI (state);
    state->pc += 1; // increment for immediate
    state->pc += 1;
    break;

    // ---------------------- LXI -----------------------------  
  case 0x01:               // LXI B, word
    state->registers[1] = opcode[1]; // little endian load
    state->registers[0] = opcode[2];
    state->pc += 2; // advance PC 2 because next 2 bytes were data
    state->pc += 1;
    break;
  case 0x11:               // LXI D, word
    state->registers[3] = opcode[1];
    state->registers[2] = opcode[2];
    state->pc += 2;
    state->pc += 1;
    break;
  case 0x21:               // LXI H, word
    state->registers[5] = opcode[1];
    state->registers[4] = opcode[2];
    state->pc += 2;
    state->pc += 1;
    break;
    // --------------------- STAX -----------------------------
  case 0x02:               // STAX B, store A indirect
    {
      uint16_t BC = (state->registers[0]<<8) | (state->registers[1]);

      state->memory[BC] = state->registers[7];
    }
    
    state->pc += 1;
    break;
  case 0x12:               // STAX D, store A indirect
    {
      uint16_t DE = (state->registers[2]<<8) | (state->registers[3]);

      state->memory[DE] = state->registers[7];
    }
    
    state->pc += 1;
    break;
 
    // --------------------- LDAX -----------------------------

  case 0x0A:               // LDAX B, load A indirect
    { // DEFINE SCOPE LOCAL TO SWITCH CASE
      uint16_t BC = (state->registers[0]<<8) | (state->registers[1]);

      // value in memory at BC
      uint8_t mem = state->memory[BC];
      
      state->registers[7] = mem;
    }
    state->pc += 1;
    break;

  case 0x1A:               // LDAX D, load A indirect
    {
      uint16_t DE = (state->registers[2]<<8) | (state->registers[3]);
      
      // value in memory at DE
      uint8_t mem = state->memory[DE];
      
      state->registers[7] = mem;
    }
    state->pc += 1;
    break;

    // ----------------------- STA -----------------------------

  case 0x32:               // STA, store A direct
    {
      uint16_t addr = (opcode[2]<<8) | (opcode[1]);
      
      state->memory[addr] = state->registers[7]; // store A at addr
      
      state->pc += 2; // for data bytes
    }
    state->pc += 1;
    break;

    // ----------------------- LDA -----------------------------

  case 0x3A:               // LDA, load A direct
    { // opcode[1] -> low_addr, opcode[2] -> high_addr
      uint16_t addr = (opcode[2]<<8) | (opcode[1]);
      
      state->registers[7] = state->memory[addr]; // store value at addr into A
      
      state->pc += 2; // for data bytes
    }
    state->pc += 1;
    break;

  case 0x22:               // SHLD, store H&L direct
    {
      uint16_t addr = (opcode[2]<<8) | (opcode[1]); // little endian

      // L stored at (HI, LOW)
      // H stored at next higher memory address
      state->memory[addr] = state->registers[5];
      state->memory[addr + 1] = state->registers[4];

      state->pc += 2; // for data bytes
    }
    state->pc += 1;
    break;
    
  case 0x2A:               // LHLD, load H&L direct
    {
      uint16_t addr = (opcode[2]<<8) | (opcode[1]); // little endian

      // L takes value stored at (HI, LOW)
      // H takes value stored at next higher memory address
      state->registers[5] = state->memory[addr];
      state->registers[4] = state->memory[addr + 1];

      state->pc += 2; // for data bytes
    }
    state->pc += 1;
    break;

  case 0xEB:               // XCHG, exchange DE HL
    {
      // H <-> D, L <-> E
      uint8_t temp = state->registers[4]; // H
      state->registers[4] = state->registers[2];
      state->registers[2] = temp;
      
      temp = state->registers[5]; // L
      state->registers[5] = state->registers[3];
      state->registers[3] = temp;
    }
    state->pc += 1;
    break;

    // -------------------- STACK OPS ---------------------------

    // --------------------- PUSH------------------------------
  case 0xC5:               // PUSH B, push register BC on stack
    state->pc += 1; // for op

    // SP-1 <- RP1
    state->memory[state->sp - 1] = state->registers[0];
    // SP-2 <- RP2
    state->memory[state->sp - 2] = state->registers[1];
    // SP <- SP - 2
    state->sp -= 2;
    
    break;
  case 0xD5:               // PUSH D, push register DE on stack
    state->pc += 1; // for op
    
    // SP-1 <- RP1
    state->memory[state->sp - 1] = state->registers[2];
    // SP-2 <- RP2
    state->memory[state->sp - 2] = state->registers[3];
    // SP <- SP - 2
    state->sp -= 2;
    break;
  case 0xE5:               // PUSH H, push register HL on stack
    state->pc += 1; // for op

    // SP-1 <- RP1
    state->memory[state->sp - 1] = state->registers[4];
    // SP-2 <- RP2
    state->memory[state->sp - 2] = state->registers[5];
    // SP <- SP - 2
    state->sp -= 2;
    break;
  case 0xF5:                 // PUSH PSW, push A and flags on stack
    {
      state->pc += 1; // for op

      // (sp - 2) <- flags,  s, z p, c, ac
      // 0 <- c, 1 <- 1, 2 <- p, 3 <- 0, 4 <- ac, 5 <- 0, 6 <- z, 7 <- s
      uint8_t flags = (state->cc->s << 7) | (state->cc->z << 6) | (0 << 5) | (state->cc->ac << 4) | (0 << 3) | (state->cc->p << 2) | (1 << 1) | (state->cc->c & 0x01);
      
      state->memory[state->sp - 2] = flags;
      
      // (sp - 1) <- A
      state->memory[state->sp - 1] = state->registers[7];
      
      // sp <- sp - 2
      state->sp -= 2;
    }
    break;
    
    // --------------------- POP --------------------------------
  case 0xC1:               // POP B, pops 16 bits off stack into BC
    state->pc += 1; // for op

    // RP1 <- (SP) + 1
    state->registers[0] = state->memory[state->sp + 1];
    // RP2 <- (SP)
    state->registers[1] = state->memory[state->sp];

    state->sp += 2; // shift stack pointer back

    break;
  case 0xD1:               // POP D, pops 16 bits off stack into DE
    state->pc += 1; // for op
    
    // RP1 <- (SP) + 1
    state->registers[2] = state->memory[state->sp + 1];
    // RP2 <- (SP)
    state->registers[3] = state->memory[state->sp];

    state->sp += 2; // shift stack pointer back

    break;
  case 0xE1:               // POP H, pops 16 bits off stack into HF
    state->pc += 1; // for op

    // RP1 <- (SP) + 1
    state->registers[4] = state->memory[state->sp + 1];
    // RP2 <- (SP)
    state->registers[5] = state->memory[state->sp];

    state->sp += 2; // shift stack pointer back

    break;

  case 0xF1:               // POP PSW, pop a and flags off stack
    state->pc += 1; // for op

    // restore flags, (flags) <- (sp), s, z p, c, ac
    state->cc->s = state->memory[state->sp] >> 7;
    state->cc->z = (state->memory[state->sp] & 0x40) >> 6;
    state->cc->p = (state->memory[state->sp] & 0x04) >> 2;
    state->cc->c = (state->memory[state->sp] & 0x01);
    state->cc->ac = (state->memory[state->sp] & 0x10) >> 4;

    // A <- (sp + 1)
    state->registers[7] = state->memory[state->sp + 1];

    // sp <- sp + 2
    state->sp += 2;

    break;

  case 0xE3:              // XTHL, exchange top of stack, H&L
    {
      uint8_t temp = state->memory[state->sp];
      // L <-> (SP)
      state->memory[state->sp] = state->registers[5];
      state->registers[5] = temp;

      // H <-> (SP + 1)
      temp = state->memory[state->sp + 1];
      state->memory[state->sp + 1] = state->registers[4]; // IMPORTANT
      state->registers[4] = temp;     
    }
    state->pc += 1;
    break;

  case 0xF9:              // SPHL, HL to stack pointer
    state->pc += 1; // for op

    // (SP) <- (H) : (L)
    state->sp = (state->registers[4] << 8) | (state->registers[5]);
    break;
  case 0x31:              // LXI SP, load immediate stack pointer
    {
      uint16_t addr = (opcode[2] << 8) | (opcode[1]); // little-endian
      state->sp = addr;
    }
    state->pc += 2;
    state->pc += 1;
    break;
    
    // -------------------- JUMP -------------------------------
  case 0xC3:              // JMP, unconditional
    // PC <- ADDR
    { // 11000011[low_addr][high_addr], where low_addr = ls 8 bits of mem addr
      state->pc += 1; // for op

      // LITTLE ENDIAN
      uint16_t addr = (opcode[2] << 8) | (opcode[1]);
      state->pc = addr;
    }
    break;
  case 0xDA:              // JC, jump on carry
    JMP_conditional(state, state->cc->c);
    break;
  case 0xD2:              // JNC, jump on no carry
    JMP_conditional(state, (state->cc->c == 0));
    break;
  case 0xCA:              // JZ, jump on zero
    JMP_conditional(state, state->cc->z);
    break;
  case 0xC2:              // JNZ, jump on no zero
    JMP_conditional(state, (state->cc->z == 0));
    break;
  case 0xF2:              // JP, jump on positive
    JMP_conditional(state, (state->cc->s == 0));
    break;
  case 0xFA:              // JM, jump on negative
    JMP_conditional(state, state->cc->s);
    break;
  case 0xEA:              // JPE, jump on parity even
    JMP_conditional(state, state->cc->p);
    break;
  case 0xE2:              // JPO, jump on parity odd
    JMP_conditional(state, (state->cc->p == 0));
    break;
  case 0xE9:              // PCHL, HL to program counter
    // PC <- HL
    {
      uint16_t addr = (state->registers[4] << 8) | (state->registers[5]);
      state->pc = addr;
    }
    break;

    // ---------------------- CALL ------------------------------
  case 0xCD:               // CALL, call unconditional
    // (SP - 1) <- PCH, (SP - 2) <- PCL, sp -= 2, PC <- addr
    // source = http://www.nj7p.org/Manuals/PDFs/Intel/9800153B.pdf
#ifdef CPU_DIAGNOSTIC
    if (((opcode[2] << 8) | (opcode[1] & 0xff)) == 5) {
      if (state->registers[1] == 9) {
	uint16_t offset = (state->registers[2] << 8) | (state->registers[3]);
	char *str = &state->memory[offset + 3];
	while (*str != '$')
	  printf("%c", *str++);
	printf("\n");
      }
      else if (state->registers[1] == 2) {
	printf ("print char routine called\n");
      }
    }
    else if (((opcode[2] << 8) | (opcode[1] & 0xff)) == 0) {
      exit(0);
    }
    else
#endif
    { 
      // for op, and two data bytes, next instruction IMPORTANT
      state->pc += 3;
      PushPC(state);
      
      uint16_t addr = (opcode[2] << 8) | (opcode[1]); // little-endian
      state->pc = addr;
    }
    break;
  case 0xDC:               // CC, call on carry
    CALL_conditional (state, state->cc->c);
    break;
  case 0xD4:               // CNC, call on no carry
    CALL_conditional (state, (state->cc->c== 0));
    break;
  case 0xCC:               // CZ, call on zero
    CALL_conditional (state, state->cc->z);
    break;
  case 0xC4:               // CNZ, call on no zero
    CALL_conditional (state, (state->cc->z == 0));
    break;
  case 0xF4:               // CP, call on positive
    CALL_conditional (state, (state->cc->s == 0));
    break;
  case 0xFC:               // CM, call on negative
    CALL_conditional (state, state->cc->s);
    break;
  case 0xEC:               // CPE, call on parity even
    CALL_conditional (state, state->cc->p);
    break;
  case 0xE4:               // CPO, call on parity odd
    CALL_conditional (state, (state->cc->p == 0));
    break;
    
    // ---------------------- RETURN ----------------------------

    // ---------------------- RET -------------------------------
  case 0xC9:               // RET, return
    // PCL(low order bits) <- sp, PCH <- sp + 1, SP <- sp + 2
    {
      uint16_t updatedPC = (state->memory[state->sp + 1] << 8) | (state->memory[state->sp]);
      
      state->pc = updatedPC;
      state->sp += 2;
    }
    break;
  case 0xD8:               // RC, return on carry
    RET_conditional (state, state->cc->c);
    break;
  case 0xD0:               // RNC, return on no carry
    RET_conditional (state, (state->cc->c == 0));
    break;
  case 0xC8:               // RZ, return on zero
    RET_conditional (state, state->cc->z);
    break;
  case 0xC0:               // RNZ, return on no zero
    RET_conditional (state, (state->cc->z == 0));
    break;
  case 0xF0:               // RP, return on positive
    RET_conditional (state, (state->cc->s == 0));
    break;
  case 0xF8:               // RM, return on negative
    RET_conditional (state, state->cc->s);
    break;
  case 0xE0:               // RPO, return on parity odd
    RET_conditional (state, (state->cc->p == 0));
    break;
  case 0xE8:               // RPE, return on parity even
    RET_conditional (state, state->cc->p);
    break;

    // ---------------------- RESTART ---------------------------

  case 0xC7:               // RST 0
  case 0xCF:               // RST 1
  case 0xD7:               // RST 2
  case 0xDF:               // RST 3
  case 0xE7:               // RST 4
  case 0xEF:               // RST 5
  case 0xF7:               // RST 6
  case 0xFF:               // RST 7
    RST(state, *opcode);
    break;
    
    // --------------- INCREMENTS/DECREMENTS --------------------

    // --------------------- INR --------------------------------
  case 0x04:               // INR B, increment register
  case 0x0C:               // INR C, increment register
  case 0x14:               // INR D, increment register
  case 0x1C:               // INR E, increment register
  case 0x24:               // INR H, increment register
  case 0x2C:               // INR L, increment register
  case 0x34:               // INR M, increment memory register
  case 0x3C:               // INR A, increment accumulator
    INR_R (state);
    state->pc += 1;
    break;

    // ---------------------- DCR -------------------------------
  case 0x05:               // DCR B, decrement register
  case 0x0D:               // DCR C, decrement register
  case 0x15:               // DCR D, decrement register
  case 0x1D:               // DCR E, decrement register
  case 0x25:               // DCR H, decrement register
  case 0x2D:               // DCR L, decrement register
  case 0x35:               // DCR M, decrement memory register
  case 0x3D:               // DCR A, decrement accumulator
    DCR_R (state);
    state->pc += 1;
    break;

    // ---------------------- INX -------------------------------
  case 0x03:               // INX B, increment BC
    {
      uint16_t BC = (state->registers[0]<<8) | (state->registers[1]);
      
      uint16_t sum = BC + 1;
      // populate bc
      state->registers[0] = (uint8_t) (sum>>8);
      state->registers[1] = (uint8_t) (sum & 0x00ff);
      // no flags affected
    }
    state->pc += 1;
    break;
  case 0x13:               // INX D, increment DE
    {
      uint16_t DE = (state->registers[2]<<8) | (state->registers[3]);
      
      uint16_t sum = DE + 1;
      // populate DE
      state->registers[2] = (uint8_t) (sum>>8);
      state->registers[3] = (uint8_t) (sum & 0x00ff);
      // no flags affected
    }
    state->pc += 1;
    break;
  case 0x23:               // INX H, increment HL
    {
      uint16_t HL = (state->registers[4]<<8) | (state->registers[5]);
      
      uint16_t sum = HL + 1;
      // populate HL
      state->registers[4] = (uint8_t) (sum>>8);
      state->registers[5] = (uint8_t) (sum & 0x00ff);
      // no flags affected
    }
    state->pc += 1;
    break;
      
    // ----------------------- DCX -----------------------------
  case 0x0B:               // DCX B, decrement BC
    {
      uint16_t BC = (state->registers[0]<<8) | (state->registers[1]);
      
      uint16_t sum = BC - 1;
      // populate bc
      state->registers[0] = (uint8_t) (sum>>8);
      state->registers[1] = (uint8_t) (sum & 0x00ff);
      // no flags affected
    }
    state->pc += 1;
    break;
  case 0x1B:               // DCX D, decrement DE
    {
      uint16_t DE = (state->registers[2]<<8) | (state->registers[3]);
      
      uint16_t sum = DE - 1;
      // populate de
      state->registers[2] = (uint8_t) (sum>>8);
      state->registers[3] = (uint8_t) (sum & 0x00ff);
      // no flags affected
    }
    state->pc += 1;
    break;
  case 0x2B:               // DCX H, decrement HL
    {
      uint16_t HL = (state->registers[4]<<8) | (state->registers[5]);
      
      uint16_t sum = HL - 1;
      // populate hl
      state->registers[4] = (uint8_t) (sum>>8);
      state->registers[5] = (uint8_t) (sum & 0x00ff);
      // no flags affected
    }
    state->pc += 1;
    break;

    // ------------------------ADDS------------------------------

    // -----------------------ADD r------------------------------
  case 0x80:               // ADD B, represented as 1000 0SSS, since B = 000
  case 0x81:               // ADD C
  case 0x82:               // ADD D
  case 0x83:               // ADD E
  case 0x84:               // ADD H
  case 0x85:               // ADD L
  case 0x86:               // ADD M, add from memory ("memory register")
  case 0x87:               // ADD accumulator
    ADD_R(state);
    state->pc += 1;
    break;
    
    // ----------------------- ADC -------------------------------
  case 0x88:               // ADC B, add to A with carry
  case 0x89:               // ADC C, add to A with carry
  case 0x8A:               // ADC D, add to A with carry
  case 0x8B:               // ADC E, add to A with carry
  case 0x8C:               // ADC H, add to A with carry
  case 0x8D:               // ADC L, add to A with carry
  case 0x8E:               // ADC M, add memory to A with carry
  case 0x8F:               // ADC A, add accumulator to A with carry
    ADC_R(state);
    state->pc += 1;
    break;

    // ---------------------- ADI --------------------------------
  case 0xC6:               // ADI byte, add immediate
    {
      uint16_t sum = (uint16_t) state->registers[7] + (uint16_t) opcode[1]; // next byte
      uint8_t lsb = (state->registers[7] & 0x0f) + (opcode[1] & 0x0f);
      
      UpdateCCAll(state, sum, lsb);
      state->registers[7] = sum & 0xff;
      state->pc += 1; // for the immediate byte
    }
    state->pc += 1;
    break;

  case 0xCE:               // ACI, add immediate to A with carry
    {
      uint16_t sum = (uint16_t) state->registers[7] + (uint16_t) opcode[1] + (uint16_t) state->cc->c;
      uint8_t lsb = (state->registers[7] & 0x0f) + ((opcode[1] + state->cc->c) & 0x0f);
      
      UpdateCCAll(state, sum, lsb);
      state->registers[7] = sum & 0xff;
      state->pc += 1;
    }
    state->pc += 1;
    break;

    // ------------------------ DAD ---------------------------
  case 0x09:              // DAD B, BC + HL --> HL, updates only carry flag
    {
      uint16_t BC = (state->registers[0]<<8) | (state->registers[1]);
      uint16_t HL = (state->registers[4]<<8) | (state->registers[5]);
      
      uint32_t sum = BC + HL;
      // populate HL
      state->registers[4] = (uint8_t) ((sum >> 8) & 0xff);
      state->registers[5] = (uint8_t) (sum & 0xff);
      
      // update carry flag
      state->cc->c = (sum > 0xffff);
    }
    state->pc += 1;
    break;
  case 0x19:              // DAD D, DE + HL --> HL, updates only carry flag
    {
      uint16_t DE = (state->registers[2]<<8) | (state->registers[3]);
      uint16_t HL = (state->registers[4]<<8) | (state->registers[5]);
      
      uint32_t sum = DE + HL;
      
      // populate HL
      state->registers[4] = (uint8_t) ((sum >> 8) & 0xff);
      state->registers[5] = (uint8_t) (sum & 0xff);
      
      // update carry flag
      state->cc->c = (sum > 0xffff);
    }
    state->pc += 1;
    break;
  case 0x29:              // DAD H, HL + HL --> HL, updates only carry flag
    {
      uint16_t HL = (state->registers[4]<<8) | (state->registers[5]);
      
      uint32_t sum = HL + HL;

      // populate HL
      state->registers[4] = (uint8_t) ((sum >> 8) & 0xff);
      state->registers[5] = (uint8_t) (sum & 0xff);
      
      // update carry flag
      state->cc->c = (sum > 0xffff);
    }
    state->pc += 1;
    break;
  case 0x39:              // DAD SP, SP + HL --> HL, updates only carry flag
    {
      uint16_t HL = (state->registers[4]<<8) | (state->registers[5]);
      
      uint32_t sum = state->sp + HL;
      // populate HL
      state->registers[4] = (uint8_t) ((sum >> 8) & 0xff);
      state->registers[5] = (uint8_t) (sum & 0xff);
      
      // update carry flag
      state->cc->c = (sum > 0xffff);
    }
    state->pc += 1;
    break;

    // ------------------- SUBTRACTS --------------------------

  case 0x90:              // SUB B, subtract register with A, goes into A
  case 0x91:              // SUB C, subtract register with A, goes into A
  case 0x92:              // SUB D, subtract register with A, goes into A
  case 0x93:              // SUB E, subtract register with A, goes into A
  case 0x94:              // SUB H, subtract register with A, goes into A
  case 0x95:              // SUB L, subtract register with A, goes into A
  case 0x96:              // SUB M, subtract register with A, goes into A
  case 0x97:              // SUB A, subtract register with A, goes into A
    SUB_R (state);
    state->pc += 1;
    break;

  case 0x98:              // SBB B, subtract register with A with carry
  case 0x99:              // SBB C, subtract register with A with carry
  case 0x9A:              // SBB D, subtract register with A with carry
  case 0x9B:              // SBB E, subtract register with A with carry
  case 0x9C:              // SBB H, subtract register with A with carry
  case 0x9D:              // SBB L, subtract register with A with carry
  case 0x9E:              // SBB M, subtract register with A with carry
  case 0x9F:              // SBB A, subtract register with A with carry
    SBB_R (state);
    state->pc += 1;
    break;

  case 0xD6:              // SUI, subtract immediate from A
    {
      uint16_t result = (uint16_t) state->registers[7] - (uint16_t) opcode[1];
      uint8_t lsb = (state->registers[7] & 0x0f) - (opcode[1] & 0x0f);
      
      UpdateCCSub(state, result, lsb);
      state->registers[7] = result & 0xff;
      state->pc += 1; // for the immediate byte
    }
    state->pc += 1;
    break;

  case 0xDE:              // SBI, subtract immediate from A with carry
    {
      uint16_t result = (uint16_t) state->registers[7] - (uint16_t) opcode[1] - (uint16_t) state->cc->c;
      uint8_t lsb = (state->registers[7] & 0x0f) - ((opcode[1] & 0x0f) + state->cc->c);
      
      UpdateCCSub(state, result, lsb);
      state->registers[7] = result & 0xff;
      state->pc += 1; // for the immediate byte
    }
    state->pc += 1;
    break;

    // ------------------- LOGICAL ----------------------------

    // ----------------------- ANA ----------------------------
  case 0xA0:              // ANA B, AND register with A, goes into A
  case 0xA1:              // ANA C, AND register with A, goes into A
  case 0xA2:              // ANA D, AND register with A, goes into A
  case 0xA3:              // ANA E, AND register with A, goes into A
  case 0xA4:              // ANA H, AND register with A, goes into A
  case 0xA5:              // ANA L, AND register with A, goes into A
  case 0xA6:              // ANA Memory, AND register with A, goes into A
  case 0xA7:              // ANA A, AND register with A, goes into A
    ANA (state);
    state->pc += 1;
    break;
    
    // ----------------------- XRA ----------------------------
  case 0xA8:              // XRA B, XOR register with A, goes into A
  case 0xA9:              // XRA C, XOR register with A, goes into A
  case 0xAA:              // XRA D, XOR register with A, goes into A
  case 0xAB:              // XRA E, XOR register with A, goes into A
  case 0xAC:              // XRA H, XOR register with A, goes into A
  case 0xAD:              // XRA L, XOR register with A, goes into A
  case 0xAE:              // XRA Memory, XOR register with A, goes into A
  case 0xAF:              // XRA A, XOR register with A, goes into A
    XRA (state);
    state->pc += 1;
    break;

    // ------------------------ ORA r  -------------------------
  case 0xB0:              // ORA B, OR register with A, goes into A
  case 0xB1:              // ORA C, OR register with A, goes into A
  case 0xB2:              // ORA D, OR register with A, goes into A
  case 0xB3:              // ORA E, OR register with A, goes into A
  case 0xB4:              // ORA H, OR register with A, goes into A
  case 0xB5:              // ORA L, OR register with A, goes into A
  case 0xB6:              // ORA M, OR register with A, goes into A
  case 0xB7:              // ORA A, OR register with A, goes into A
    ORA (state);
    state->pc += 1;
    break;

    
    // ------------------------ CMP ------------------------------
  case 0xB8:              // CMP B, compare register with A
  case 0xB9:              // CMP C, compare register with A
  case 0xBA:              // CMP D, compare register with A
  case 0xBB:              // CMP E, compare register with A
  case 0xBC:              // CMP H, compare register with A
  case 0xBD:              // CMP L, compare register with A
  case 0xBE:              // CMP M, compare memory with A
  case 0xBF:              // CMP A, compare register with A
    CMP (state);
    state->pc += 1;
    break;

    // ----------------------- ANI ----------------------------
  case 0xE6:              // ANI, and immediate with A
    {
      uint8_t result = state->registers[7] & opcode[1];
      
      state->registers[7] = result;
      
      UpdateCCSimple(state, result);
      state->cc->c = 0; // carry is zeroed
      state->cc->ac = 0; // ac is zeroed
      
      state->pc += 1;
    }
    state->pc += 1;
    break;

  case 0xEE:              // XRI, XOR immediate with A
    {
      uint8_t result = state->registers[7] ^ opcode[1];
      
      state->registers[7] = result;
      
      UpdateCCSimple(state, result);
      state->cc->c = 0; // carry is zeroed
      state->cc->ac = 0; // ac is zeroed
      
      state->pc += 1; // data byte
    }
    state->pc += 1;
    break;
    
  case 0xF6:              // ORI, OR immediate with A
    {
      uint8_t result = state->registers[7] | opcode[1];
      
      state->registers[7] = result;
      
      UpdateCCSimple(state, result);
      state->cc->c = 0; // carry is zeroed
      state->cc->ac = 0; // ac is zeroed
      
      state->pc += 1; // data byte
    }
    state->pc += 1;
    break;

    // ------------------------ CPI ---------------------------
  case 0xFE:              // CPI, compare immediate with A
    // condition bits set by (A) - Data
    {
      uint16_t result = ((uint16_t) state->registers[7] - (uint16_t) opcode[1]);
      uint8_t lsb = (state->registers[7] & 0x0f) - (opcode[1] & 0x0f);
      
      UpdateCCAll(state, result, lsb);
      if (state->registers[7] >= opcode[1])
	state->cc->c = 0;
      else if (state->registers[7] < opcode[1])
	state->cc->c = 1;
	  
      state->pc += 1;
    }
    state->pc += 1;
    break;
    
    // ------------------- ROTATE -----------------------------
  case 0x07:              // RLC, rotate A left
    {
      uint8_t result;
      uint8_t a7 = (state->registers[7] & 0x80) >> 7;

      // set carry - a7
      state->cc->c = a7;

      // rotate A left
      // rotate
      result = state->registers[7] << 1;
      state->registers[7] = result;

      // A0 <- A7
      state->registers[7] &= 0xfe; // clear first bit
      state->registers[7] |= (a7 & 0x01);
    }
    state->pc += 1;
    break;
  case 0x0F:              // RRC, rotate A right
    {
      uint8_t result;
      uint8_t a0 = state->registers[7] & 0x01;

      // set carry = A0
      state->cc->c = a0;
      
      // rotate A right
      // rotate
      result = state->registers[7] >> 1;
      state->registers[7] = result;
      
      // A7 <- A0
      state->registers[7] &= 0x7f;         // clear last bit
      state->registers[7] |= (a0 << 7);    // update last bit
    }
    state->pc += 1;
    break;
  case 0x17:              // RAL, rotate A left through carry
    {
      uint8_t result;
      uint8_t a7 = (state->registers[7] & 0x80) >> 7; // save a7

      // rotate A left
      // rotate
      result = state->registers[7] << 1;
      state->registers[7] = result;

      // A0 <- carry
      state->registers[7] &= 0xfe; // clear first bit
      state->registers[7] |= (state->cc->c & 0x01);

      // set carry - a7
      state->cc->c = a7;
    }
    state->pc += 1;
    break;
  case 0x1F:              // RAR, rotate A right through carry
    { // 01101010 (1) -> 10110101 (0) , where () is carry
      uint8_t result;
      uint8_t a0 = state->registers[7] & 0x01; // save a0
  
      // rotate A right
      // rotate
      result = state->registers[7] >> 1;
      state->registers[7] = result;
      
      // A7 <- A0
      state->registers[7] &= 0x7f;         // clear last bit
      state->registers[7] |= (state->cc->c << 7);    // update last bit

      // set carry = A0
      state->cc->c = a0;
    }
    state->pc += 1;
    break;

    // -------------------- SPECIALS ---------------------------
  case 0x2F:              // CMA, complement A
    state->registers[7] ^= 0xff;
    state->pc += 1;
    break;
  case 0x37:              // STC, set carry
    state->cc->c = 1;
    state->pc += 1;
    break;
  case 0x3F:              // CMC, complement carry
    state->cc->c ^= 0x01;
    state->pc += 1;
    break;   
  case 0x27:              // DAA, decimal adjust A
    {
      uint16_t result = state->registers[7];
      if (state->cc->ac || ((state->registers[7] & 0x0f) > 9)) {
        result += 6;
	uint8_t lsb = (state->registers[7] & 0x0f) + 6;
	
	state->registers[7] = (uint8_t) (result & 0xff);

	if (lsb > 0x0f) // carry out from lsb
	  state->cc->ac = 1;
	else
	  state->cc->ac = 0;
      }
      if (state->cc->c || (((state->registers[7] >> 4) & 0x0f) > 9)) {
	uint8_t gsb = (state->registers[7] >> 4) + 6;
	
	// reset result with new gsb
	result = (gsb << 4) | (state->registers[7] & 0x0f);
	state->registers[7] = (uint8_t) (result & 0xff);

	UpdateCCSimple(state, gsb);
	if (gsb > 0x0f) // carry out from gsb
	  state->cc->c = 1;	
      }
      
    }
    state->pc += 1;
    break;
    

    // ------------------ INPUT/OUTPUT -------------------------
  case 0xDB:              // IN, input
    // (A) <- input device
    // data byte = port
    {
      uint8_t port = opcode[1];

      state->registers[7] = (*state->drivers->in[port]) ();
      state->pc += 1;
      state->pc += 1;
    }
    break;
  case 0xD3:              // OUT, output
    // output device <- (A)
    {
      uint8_t port = opcode[1];

      void (*p) (uint8_t i) = state->drivers->out[port];

      (*p) (state->registers[7]);
      state->pc += 1;
      state->pc += 1;
    }
    break;

    // --------------------- CONTROL -----------------------------
  case 0xFB:              // EI, enable interrupts
    state->IE = 1;
    state->pc += 1;
    break;
  case 0xF3:              // DI, disable interrupts
    state->IE = 0;
    state->pc += 1;
    break;
  case 0x76:              // HLT, halt
    printf ("Recieved halt");
    exit(0);
    break;
  default:
    UnimplementedInstruction(state);
  }

  //#ifdef DEBUG
  //if (state->pc == 0x0ABB)
  //  flag = 1;
  if (flag) {
    
  //  PRINT CURRENT INSTRUCTION AND STATE
  printf("\t CURRENT INSTRUCTION: $%02x\n", *opcode);
  
  printf("\t CURRENT STATE: \n");
  
  printf("\tC=%d, P=%d, S=%d, Z = %d, IE = %d\n", state->cc->c, state->cc->p, state->cc->s, state->cc->z, state->IE);
  printf("\tA $%02x B $%02x C $%02x D $%02x E $%02x H $%02x L $%02x SP %04x PC  %04x\n", state->registers[7], state->registers[0], state->registers[1], state->registers[2], state->registers[3], state->registers[4], state->registers[5], state->sp, state->pc);
  
  printf("\n");
  
  fflush(stdout);
  }
  //#endif

  #ifdef INSTRUCTION_DEBUGGING
  if (state->sp < 0x22ff && state->pc != 0) {
    fprintf(stderr, "Stack pointer is too low: %i on instruction $%02x, pc $%04x\n", state->sp, *opcode, state->pc);
    times++;
    if (times > 15)
      exit(0);
  }
  #endif
  /*
  if (state->pc == 0x1A36) {
    printf("On 1a36, B $%02x, zero=%d\n", state->registers[0], state->cc->z);
  }
  else if (state->pc == 0x1A3A) {
    printf("on 1a3A, B $%02x, zero=%d, pc $%04x\n", state->registers[0], state->cc->z, state->pc);
  }
  */
  //printf("Stack pointer : $%04x, pc : $%04x\n", state->sp, state->pc);
}

void Emulate8080State(State8080_T state) {
  unsigned char *opcode;
  
  // get opcode from memory at PC, PC steps away from start of memory
  opcode = &state->memory[state->pc];

  #ifdef DEBUG
  printf("Executing $%02x", *opcode);
  fflush(stdout);
  #endif

  Emulate8080Op(state, opcode);
}

ConditionCodes_T ConditionCodes_init () {
  ConditionCodes_T cc = (struct ConditionCodes *) malloc (sizeof (struct ConditionCodes));

  cc->c = 0;
  cc->p = 0;
  cc->z = 0;
  cc->ac = 0;
  cc->s = 0;
  cc->pad = 0;

  return cc;
}

void ConditionCodes_free (ConditionCodes_T cc) {
  free(cc);
}

uint8_t empty_IN () {
  return 0;
}

void empty_OUT (uint8_t ac) {
  return;
}

Drivers_T Drivers_init () {
  Drivers_T drivers = (struct Drivers *) malloc (sizeof (struct Drivers));

  drivers->in[0] = &empty_IN;
  drivers->in[1] = &empty_IN;
  drivers->in[2] = &empty_IN;
  drivers->in[3] = &empty_IN;
  drivers->in[4] = &empty_IN;
  drivers->in[5] = &empty_IN;
  drivers->in[6] = &empty_IN;
  drivers->in[7] = &empty_IN;

  drivers->out[0] = &empty_OUT;
  drivers->out[1] = &empty_OUT;
  drivers->out[2] = &empty_OUT;
  drivers->out[3] = &empty_OUT;
  drivers->out[4] = &empty_OUT;
  drivers->out[5] = &empty_OUT;
  drivers->out[6] = &empty_OUT;
  drivers->out[7] = &empty_OUT;

  return drivers;
}

void Drivers_free (Drivers_T drivers) {
  free(drivers);
}

State8080_T State8080_init () {
  State8080_T state = (struct State8080 *) malloc (sizeof (struct State8080));

  state->registers[0] = 0;
  state->registers[1] = 0;
  state->registers[2] = 0;
  state->registers[3] = 0;
  state->registers[4] = 0;
  state->registers[5] = 0;
  state->registers[6] = 0;
  state->registers[6] = 0;

  state->sp = 0;
  state->pc = 0;
  state->memory = (uint8_t *) calloc (65535, sizeof (uint8_t));
  state->interrupts = (uint8_t *) calloc (1, sizeof(uint8_t));

  state->cc = ConditionCodes_init();
  state->drivers = Drivers_init();

  state->IE = 0;

  return state;
}

void State8080_free (State8080_T state) {
  free(state->memory);
  free(state->interrupts);
  ConditionCodes_free(state->cc);
  Drivers_free(state->drivers);
  
  free(state);
}

void State8080_load_mem(State8080_T state, int start, size_t end, unsigned char *buffer) {
  for (int i = start; i < end && i < 8192; i++) {
    state->memory[i] = (uint8_t) *buffer;
    buffer++;
  }
}

void State8080_config_drivers_in_port(State8080_T state, uint8_t (*in) (), uint8_t port) {
  state->drivers->in[port] = in;
}

void State8080_config_drivers_out_port(State8080_T state, void (*out) (uint8_t), uint8_t port) {
  state->drivers->out[port] = out;
}

void State8080_config_drivers_default(State8080_T state, Drivers_T drivers) {
  state->drivers = drivers;
}
void config_drivers_in_port(Drivers_T drivers, uint8_t (*in) (), uint8_t port) {
  drivers->in[port] = in;
}

void config_drivers_out_port(Drivers_T drivers, void (*out) (uint8_t), uint8_t port) {
  drivers->out[port] = out;
}

uint8_t State8080_ie(State8080_T state) {
  return state->IE;
}

void State8080_setIE(State8080_T state, uint8_t val) {
  state->IE = val;
}

void State8080_pushInterrupt(State8080_T state, uint8_t int_num) {
  state->interrupts[0] = int_num;
}

uint8_t State8080_popInterrupt(State8080_T state) {
  return state->interrupts[0];
}

// ----------------------- CYCLES INFO ON CPU ---------------------
int op_clockCycles(State8080_T state) {
  unsigned char *opcode;
  opcode = &state->memory[state->pc];
  
  switch (*opcode) {
  case 0x00:               // NOP
    return 4;      
    // --------------- MOVE/LOADS/STORES ----------------------
    
    // TODO: FORMAT THESE COMMENTS FOR MOV PROPERLY
    // -------------------- MOV -------------------------------
  case 0x40:               // MOV B, B
  case 0x41:               // MOV B, C
  case 0x42:               // MOV B, D
  case 0x43:               // MOV B, E
  case 0x44:               // MOV B, H
  case 0x45:               // MOV B, L
    return 5;
  case 0x46:               // MOV B, Memory
    return 7;
  case 0x47:               // MOV B, Accumulator
  case 0x48:               // MOV C, B
  case 0x49:               // MOV C, C
  case 0x4A:               // MOV B, C
  case 0x4B:               // MOV B, D
  case 0x4C:               // MOV B, E
  case 0x4D:               // MOV B, C
    return 5;
  case 0x4E:               // MOV C, Memory
    return 7;
  case 0x4F:               // MOV B, E
  case 0x50:               // MOV B, 
  case 0x51:               // MOV B, C
  case 0x52:               // MOV B, D
  case 0x53:               // MOV B, E
  case 0x54:               // MOV B, H
  case 0x55:               // MOV B, L
    return 5;
  case 0x56:               // MOV D, Memory
    return 7;
  case 0x57:               // MOV B, Accumulator
  case 0x58:
  case 0x59:               // MOV B, E
  case 0x5A:               // MOV B, C
  case 0x5B:               // MOV B, D
  case 0x5C:               // MOV B, E
  case 0x5D:               // MOV B, C
    return 5;
  case 0x5E:               // MOV E, Memory
    return 7;
  case 0x5F:               // MOV B, E
  case 0x60:               // MOV B, 
  case 0x61:               // MOV B, C
  case 0x62:               // MOV B, D
  case 0x63:               // MOV B, E
  case 0x64:               // MOV B, H
  case 0x65:               // MOV B, L
    return 5;
  case 0x66:               // MOV B, Memory
    return 7;
  case 0x67:               // MOV B, Accumulator
  case 0x68:
  case 0x69:               // MOV B, E
  case 0x6A:               // MOV B, C
  case 0x6B:               // MOV B, D
  case 0x6C:               // MOV B, E
  case 0x6D:               // MOV B, C
    return 5;
  case 0x6E:               // MOV B, D
    return 7;
  case 0x6F:               // MOV B,
  case 0x70:               // MOV M, B
  case 0x71:               // MOV M, C
  case 0x72:               // MOV M, D
  case 0x73:               // MOV M, E
  case 0x74:               // MOV M, H
  case 0x75:               // MOV M, L
  case 0x77:               // MOV M, Accumulator
    return 7;
  case 0x78:
  case 0x79:               // MOV B, E
  case 0x7A:               // MOV B, C
  case 0x7B:               // MOV B, D
  case 0x7C:               // MOV B, E
  case 0x7D:               // MOV B, C
    return 5;
  case 0x7E:               // MOV B, D
    return 7;
  case 0x7F:               // MOV B, E
    return 5;

    // --------------------- MVI ------------------------------
  case 0x06:               // MVI B, move immediate to B
  case 0x0E:               // MVI C, move immediate to C
  case 0x16:               // MVI D, move immediate to D
  case 0x1E:               // MVI E, move immediate to E
  case 0x26:               // MVI H, move immediate to H
  case 0x2E:               // MVI L, move immediate to L
    return 7;
  case 0x36:               // MVI memory, move immediate to memory
    return 10;
  case 0x3E:               // MVI A, move immediate to accumulator
    return 7;

    // ---------------------- LXI -----------------------------  
  case 0x01:               // LXI B, word
  case 0x11:               // LXI D, word
  case 0x21:               // LXI H, word
    return 10;
    // --------------------- STAX -----------------------------
  case 0x02:               // STAX B, store A indirect
  case 0x12:               // STAX D, store A indirect
    return 7;
  
    // --------------------- LDAX -----------------------------

  case 0x0A:               // LDAX B, load A indirect
  case 0x1A:               // LDAX D, load A indirect
    return 7;

    // ----------------------- STA -----------------------------

  case 0x32:               // STA, store A direct
    return 13;

    // ----------------------- LDA -----------------------------

  case 0x3A:               // LDA, load A direct
    return 13;

  case 0x22:               // SHLD, store H&L direct 
  case 0x2A:               // LHLD, load H&L direct
    return 16;

  case 0xEB:               // XCHG, exchange DE HL
    return 4;

    // -------------------- STACK OPS ---------------------------

    // --------------------- PUSH------------------------------
  case 0xC5:               // PUSH B, push register BC on stack
  case 0xD5:               // PUSH D, push register DE on stack
  case 0xE5:               // PUSH H, push register HL on stack 
  case 0xF5:               // PUSH PSW, push A and flags on stack
    return 11;
    
    // --------------------- POP --------------------------------
  case 0xC1:               // POP B, pops 16 bits off stack into BC
  case 0xD1:               // POP D, pops 16 bits off stack into DE
  case 0xE1:               // POP H, pops 16 bits off stack into HF
  case 0xF1:               // POP PSW, pop a and flags off stack
    return 10;

  case 0xE3:              // XTHL, exchange top of stack, H&L
    return 18;
  case 0xF9:              // SPHL, HL to stack pointer
    return 5;
  case 0x31:              // LXI SP, load immediate stack pointer
    return 10;
    
    // -------------------- JUMP -------------------------------
  case 0xC3:              // JMP, unconditional
  case 0xDA:              // JC, jump on carry
  case 0xD2:              // JNC, jump on no carry
  case 0xCA:              // JZ, jump on zero
  case 0xC2:              // JNZ, jump on no zero
  case 0xF2:              // JP, jump on positive
  case 0xFA:              // JM, jump on negative
  case 0xEA:              // JPE, jump on parity even
  case 0xE2:              // JPO, jump on parity odd
    return 10;
  case 0xE9:              // PCHL, HL to program counter
    return 5;
    // ---------------------- CALL ------------------------------
  case 0xCD:               // CALL, call unconditional
    return 17;
  case 0xDC:               // CC, call on carry
    if (state->cc->c)
      return 17;
    return 11;
  case 0xD4:               // CNC, call on no carry
    if (state->cc->c== 0)
      return 17;
    return 11;
  case 0xCC:               // CZ, call on zero
    if (state->cc->z)
      return 17;
    return 11;
  case 0xC4:               // CNZ, call on no zero
    if (state->cc->z== 0)
      return 17;
    return 11;
  case 0xF4:               // CP, call on positive
    if (state->cc->s== 0)
      return 17;
    return 11;
  case 0xFC:               // CM, call on negative
    if (state->cc->s)
      return 17;
    return 11;
  case 0xEC:               // CPE, call on parity even
    if (state->cc->p)
      return 17;
    return 11;
  case 0xE4:               // CPO, call on parity odd
    if (state->cc->p== 0)
      return 17;
    return 11;
    
    // ---------------------- RETURN ----------------------------

    // ---------------------- RET -------------------------------
  case 0xC9:               // RET, return
    return 10;
  case 0xD8:               // RC, return on carry
    if (state->cc->c)
      return 11;
    return 5;
  case 0xD0:               // RNC, return on no carry
    if (state->cc->c == 0)
      return 11;
    return 5;
  case 0xC8:               // RZ, return on zero
    if (state->cc->z)
      return 11;
    return 5;
  case 0xC0:               // RNZ, return on no zero
    if (state->cc->z == 0)
      return 11;
    return 5;
  case 0xF0:               // RP, return on positive
    if (state->cc->s == 0)
      return 11;
    return 5;
  case 0xF8:               // RM, return on negative
    if (state->cc->s)
      return 11;
    return 5;
  case 0xE0:               // RPO, return on parity odd
    if (state->cc->p == 0)
      return 11;
    return 5;
  case 0xE8:               // RPE, return on parity even
    if (state->cc->p)
      return 11;
    return 5;

    // ---------------------- RESTART ---------------------------

  case 0xC7:               // RST 0
  case 0xCF:               // RST 1
  case 0xD7:               // RST 2
  case 0xDF:               // RST 3
  case 0xE7:               // RST 4
  case 0xEF:               // RST 5
  case 0xF7:               // RST 6
  case 0xFF:               // RST 7
    return 11;
    
    // --------------- INCREMENTS/DECREMENTS --------------------

    // --------------------- INR --------------------------------
  case 0x04:               // INR B, increment register
  case 0x0C:               // INR C, increment register
  case 0x14:               // INR D, increment register
  case 0x1C:               // INR E, increment register
  case 0x24:               // INR H, increment register
  case 0x2C:               // INR L, increment register
    return 5;
  case 0x34:               // INR M, increment memory register
    return 10;
  case 0x3C:               // INR A, increment accumulator
    return 5;

    // ---------------------- DCR -------------------------------
  case 0x05:               // DCR B, decrement register
  case 0x0D:               // DCR C, decrement register
  case 0x15:               // DCR D, decrement register
  case 0x1D:               // DCR E, decrement register
  case 0x25:               // DCR H, decrement register
  case 0x2D:               // DCR L, decrement register
    return 5;
  case 0x35:               // DCR M, decrement memory register
    return 10;
  case 0x3D:               // DCR A, decrement accumulator
    return 5;

    // ---------------------- INX -------------------------------
  case 0x03:               // INX B, increment BC
  case 0x13:               // INX D, increment DE
  case 0x23:               // INX H, increment HL
    return 5;
      
    // ----------------------- DCX -----------------------------
  case 0x0B:               // DCX B, decrement BC
  case 0x1B:               // DCX D, decrement DE
  case 0x2B:               // DCX H, decrement HL
    return 5;

    // ------------------------ADDS------------------------------

    // -----------------------ADD r------------------------------
  case 0x80:               // ADD B, represented as 1000 0SSS, since B = 000
  case 0x81:               // ADD C
  case 0x82:               // ADD D
  case 0x83:               // ADD E
  case 0x84:               // ADD H
  case 0x85:               // ADD L
    return 4;
  case 0x86:               // ADD M, add from memory ("memory register")
    return 7;
  case 0x87:               // ADD accumulator
    return 4;
    
    // ----------------------- ADC -------------------------------
  case 0x88:               // ADC B, add to A with carry
  case 0x89:               // ADC C, add to A with carry
  case 0x8A:               // ADC D, add to A with carry
  case 0x8B:               // ADC E, add to A with carry
  case 0x8C:               // ADC H, add to A with carry
  case 0x8D:               // ADC L, add to A with carry
    return 4;
  case 0x8E:               // ADC M, add memory to A with carry
    return 7;
  case 0x8F:               // ADC A, add accumulator to A with carry
    return 4;
    
    // ---------------------- ADI --------------------------------
  case 0xC6:               // ADI byte, add immediate
  case 0xCE:               // ACI, add immediate to A with carry
    return 7;

    // ------------------------ DAD ---------------------------
  case 0x09:              // DAD B, BC + HL --> HL, updates only carry flag
  case 0x19:              // DAD D, DE + HL --> HL, updates only carry flag
  case 0x29:              // DAD H, HL + HL --> HL, updates only carry flag
  case 0x39:              // DAD SP, SP + HL --> HL, updates only carry flag
    return 10;

    // ------------------- SUBTRACTS --------------------------
  case 0x90:              // SUB B, subtract register with A, goes into A
  case 0x91:              // SUB C, subtract register with A, goes into A
  case 0x92:              // SUB D, subtract register with A, goes into A
  case 0x93:              // SUB E, subtract register with A, goes into A
  case 0x94:              // SUB H, subtract register with A, goes into A
  case 0x95:              // SUB L, subtract register with A, goes into A
    return 4;
  case 0x96:              // SUB M, subtract register with A, goes into A
    return 7;
  case 0x97:              // SUB A, subtract register with A, goes into A
    return 4;

  case 0x98:              // SBB B, subtract register with A with carry
  case 0x99:              // SBB C, subtract register with A with carry
  case 0x9A:              // SBB D, subtract register with A with carry
  case 0x9B:              // SBB E, subtract register with A with carry
  case 0x9C:              // SBB H, subtract register with A with carry
  case 0x9D:              // SBB L, subtract register with A with carry
    return 4;
  case 0x9E:              // SBB M, subtract register with A with carry
    return 7;
  case 0x9F:              // SBB A, subtract register with A with carry
    return 4;

  case 0xD6:              // SUI, subtract immediate from A
  case 0xDE:              // SBI, subtract immediate from A with carry
    return 7;
    
    // ------------------- LOGICAL ----------------------------

    // ----------------------- ANA ----------------------------
  case 0xA0:              // ANA B, AND register with A, goes into A
  case 0xA1:              // ANA C, AND register with A, goes into A
  case 0xA2:              // ANA D, AND register with A, goes into A
  case 0xA3:              // ANA E, AND register with A, goes into A
  case 0xA4:              // ANA H, AND register with A, goes into A
  case 0xA5:              // ANA L, AND register with A, goes into A
    return 4;
  case 0xA6:              // ANA Memory, AND register with A, goes into A
    return 7;
  case 0xA7:              // ANA A, AND register with A, goes into A
    return 4;

    // ----------------------- XRA ----------------------------
  case 0xA8:              // XRA B, XOR register with A, goes into A
  case 0xA9:              // XRA C, XOR register with A, goes into A
  case 0xAA:              // XRA D, XOR register with A, goes into A
  case 0xAB:              // XRA E, XOR register with A, goes into A
  case 0xAC:              // XRA H, XOR register with A, goes into A
  case 0xAD:              // XRA L, XOR register with A, goes into A
    return 4;
  case 0xAE:              // XRA Memory, XOR register with A, goes into A
    return 7;
  case 0xAF:              // XRA A, XOR register with A, goes into A
    return 4;

  case 0xB0:              // ORA B, OR register with A, goes into A
  case 0xB1:              // ORA C, OR register with A, goes into A
  case 0xB2:              // ORA D, OR register with A, goes into A
  case 0xB3:              // ORA E, OR register with A, goes into A
  case 0xB4:              // ORA H, OR register with A, goes into A
  case 0xB5:              // ORA L, OR register with A, goes into A
    return 4;
  case 0xB6:              // ORA M, OR register with A, goes into A
    return 7;
  case 0xB7:              // ORA A, OR register with A, goes into A
    return 4;

       // ------------------------ CMP ------------------------------
  case 0xB8:              // CMP B, compare register with A
  case 0xB9:              // CMP C, compare register with A
  case 0xBA:              // CMP D, compare register with A
  case 0xBB:              // CMP E, compare register with A
  case 0xBC:              // CMP H, compare register with A
  case 0xBD:              // CMP L, compare register with A
    return 4;
  case 0xBE:              // CMP M, compare memory with A
    return 7;
  case 0xBF:              // CMP A, compare register with A
    return 4;
    
    // ----------------------- ANI ----------------------------
  case 0xE6:              // ANI, and immediate with A
  case 0xEE:              // XRI, XOR immediate with A 
  case 0xF6:              // ORI, OR immediate with A

    // ------------------------ CPI ---------------------------
  case 0xFE:              // CPI, compare immediate with A
    return 7;
    
    // ------------------- ROTATE -----------------------------
  case 0x07:              // RLC, rotate A left
  case 0x0F:              // RRC, rotate A right
  case 0x17:              // RAL, rotate A left through carry
  case 0x1F:              // RAR, rotate A right through carry
    return 4;
    
    // -------------------- SPECIALS --------------------------- 
  case 0x2F:              // CMA, complement A
  case 0x37:              // STC, set carry
  case 0x3F:              // CMC, complement carry
  case 0x27:              // DAA, decimal adjust A
    return 4;
    // ------------------ INPUT/OUTPUT -------------------------
    // TODO: THESE ARE TEMPORARY PLACE HOLDERS FOR THESE FUNCTIONS
    // SHOULD BE MOVED OUTSIDE
  case 0xDB:              // IN, input
  case 0xD3:              // OUT, output
    return 10;

    // --------------------- CONTROL -----------------------------
  case 0xFB:              // EI, enable interrupts
  case 0xF3:              // DI, disable interrupts
    return 4;
  case 0x76:              // HLT, halt
    return 7;
  default:
    UnimplementedInstruction(state);
  }
}

uint8_t *pointerToMemoryAt(State8080_T state, uint16_t addr) {
  uint8_t *memory = &state->memory[addr];

  return memory;
}
