/*
  File name : CPU.c
  
  Description : contains the bulk of the information for the 8080 cpu
  Author : Antony Toron

 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "CPU.h"

struct ConditionCodes { // specifying bit count (1 bit flags/flip flops)
  uint8_t z : 1;   // zero flag
  uint8_t s : 1;   // sign flag (0 = POSITIVE, 1 = NEGATIVE)
  uint8_t p : 1;   // parity flag (0 = ODD, 1 = EVEN)
  uint8_t c : 1;   // carry flag (0 = no carry, 1 = carry)
  uint8_t ac : 1;  // auxiliary carry (carry out of 3 bit accumulator - DAA) 
  uint8_t pad : 3; // ??
  // IE: flip flop for interrupt enabled
};

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
};


static void UnimplementedInstruction(State8080_T state) {
  // PC will have been incremented, so it should be decremented back

  fprintf (stderr, "Unimplemented instruction\n");
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

static void UpdateCCZeroSignParity(State8080_T state, uint16_t result) {
  state->cc->z = ((result & 0xff) == 0);
  state->cc->s = ((result & 0x80) != 0);
  state->cc->p = Parity(result & 0xff);
}

static uint8_t GetRegister(State8080_T state, uint8_t reg) {
  switch (reg) {
  case 0x06: ;
    // H = high order bits, L = lower order bits, so shift H 8 bits up
    uint16_t offset = (state->registers[4]<<8) | (state->registers[5]);
    return state->memory[offset]; break;
  case 0x07:
    return state->registers[7]; break;
  default:
    if (reg > 7) {
      fprintf (stderr, "Could not find register in get register");
      exit(1);
    }
    return state->registers[reg];
  }
}

static uint8_t AddToRegister (State8080_T state, uint8_t reg, uint8_t add) {
  switch (reg) {
  case 0x06: ;
    // H = high order bits, L = lower order bits, so shift H 8 bits up
    uint16_t offset = (state->registers[4]<<8) | (state->registers[5]);
    state->memory[offset] += add; return state->memory[offset]; break;
  default:
    if (reg > 7) { 
      fprintf(stderr, "Could not find this register in addtoreg");
      exit(1);
    }
    state->registers[reg] += add; return state->registers[reg]; break;
  }
}

static void MOV (State8080_T state) {
  unsigned char *opcode = &state->memory[state->pc];
  uint8_t dest = ((*opcode) & 0x38)>>3;
  uint8_t source = (*opcode) & 0x07;

  if (dest == 6 || source == 6) {
    if (dest == 6 && source != 6) {
      uint16_t offset = (state->registers[4]<<8) | (state->registers[5]);
      state->memory[offset] = state->registers[source];
    }
    else if (dest != 6 && source == 6) {
      uint16_t offset = (state->registers[4]<<8) | (state->registers[5]);
      state->registers[source] = state->memory[offset];
    }
    else {
      uint16_t offset = (state->registers[4]<<8) | (state->registers[5]);
      state->memory[offset] = state->memory[offset];
    }
  }
  else {
    state->registers[dest] = state->registers[source];
  }
}

static void MVI (State8080_T state) {
  unsigned char *opcode = &state->memory[state->pc];
  uint8_t dest = ((*opcode) & 0x38)>>3;

  if (dest == 6) {
    uint16_t offset = (state->registers[4]<<8) | (state->registers[5]);
    state->memory[offset] = opcode[1];
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
  
  UpdateCCSimple(state, sum);
  state->registers[7] = sum & 0xff;              // update accumulator
}

// ADD register r from opcode to accumulator (a) with carry
static void ADC_R (State8080_T state) {
  unsigned char *opcode = &state->memory[state->pc];
  uint8_t reg = (*opcode) & 0x07;
  uint8_t reg_value = GetRegister(state, reg);
  uint16_t sum = (uint16_t) state->registers[7] + (uint16_t) reg_value + (uint16_t) state->cc->c;
  
  UpdateCCSimple(state, sum);
  state->registers[7] = sum & 0xff;              // update accumulator
}

// ANA register r, r & a -> a, set sign, zero, parity. carry = 0
static void ANA (State8080_T state) {
  uint8_t result;
  unsigned char *opcode = &state->memory[state->pc];
  uint8_t reg = (*opcode) & 0x07;

  if (reg == 6) {
    uint16_t offset = (state->registers[4]<<8) | (state->registers[5]);
    result = state->registers[7] & state->memory[offset];
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
    uint16_t offset = (state->registers[4]<<8) | (state->registers[5]);
    result = state->registers[7] ^ state->memory[offset];
    state->registers[7] = result;
  }
  else {
    result = state->registers[7] ^ state->registers[reg];
    state->registers[7] = result;
  }

  UpdateCCSimple(state, result);
  state->cc->c = 0; // set carry to 0 just in case
}


// INR register r from opcode
static void INR_R (State8080_T state) {
  unsigned char *opcode = &state->memory[state->pc];
  uint8_t reg = ((*opcode) & 0x38)>>3;

  // source for CC info: http://altairclone.com/downloads/manuals/8080%20Programmers%20Manual.pdf vii

  switch (reg) {
  case 0x06: ;
    // H = high order bits, L = lower order bits, so shift H 8 bits up
    uint16_t offset = (state->registers[4]<<8) | (state->registers[5]);
    state->memory[offset]++; UpdateCCZeroSignParity(state, state->memory[offset]); break;
  default:
    if (reg > 7) {
      fprintf(stderr, "Could not find this register in increg");
      exit(1);
    }
    state->registers[reg]++;
    UpdateCCZeroSignParity(state, state->registers[reg]);
    break;
  }
}

// DCR register r from opcode
static void DCR_R (State8080_T state) {
  unsigned char *opcode = &state->memory[state->pc];
  uint8_t reg = ((*opcode) & 0x38)>>3;

  switch (reg) {
  case 0x06: ;
    // H = high order bits, L = lower order bits, so shift H 8 bits up
    uint16_t offset = (state->registers[4]<<8) | (state->registers[5]);
    state->memory[offset]--; UpdateCCZeroSignParity(state, state->memory[offset]);
    break;
  default:
    if (reg > 7) {
      fprintf(stderr, "Could not find this register in addtoreg");
      exit(1);
    }
    state->registers[reg]--;
    UpdateCCZeroSignParity(state, state->registers[reg]);
    break;
  }
}
 
static void JMP_conditional (State8080_T state, uint8_t condition) {
   unsigned char *opcode = &state->memory[state->pc];
   if (condition) {
     uint16_t addr = (opcode[1] << 8) | (opcode[2] & 0xff);
     state->pc = state->memory[addr];
   }
   else
     state->pc += 2;
}

static void RET_conditional (State8080_T state, uint8_t condition) {
  unsigned char *opcode = &state->memory[state->pc];
  if (condition) {
    // PCL(low order bits) <- sp, PCH <- sp + 1, SP <- sp + 2
    uint16_t updatedPC = (state->memory[state->sp] << 8) | (state->memory[state->sp + 1] && 0xff);
    
    state->pc = updatedPC;
    state->sp += 2;
  }
}

static void CALL_conditional (State8080_T state, uint8_t condition) {
  unsigned char *opcode = &state->memory[state->pc];
  if (condition) {
    // (sp - 1) <- PCH, (sp - 2) <- PCL, SP += 2, PC <- addr 
    state->memory[state->sp - 1] = (state->pc >> 8) & 0xff;
    state->memory[state->sp - 2] = (uint8_t) (state->pc & 0x00ff);

    state->sp += 2;
    state->pc = (opcode[1] << 8) | (opcode[2] & 0xff); // should this be reversed?
  }
  else
    state->pc += 2; // for data bytes
}

uint8_t MachineIN (State8080_T state, uint8_t port) {
  return 0; // placeholder
}

void MachineOUT (State8080_T state, uint8_t port) {
  return; // placeholder
}

void Emulate8080Op(State8080_T state) {
  unsigned char *opcode;
  
  // get opcode from memory at PC, PC steps away from start of memory
  opcode = &state->memory[state->pc];
  
  switch (*opcode) {
  case 0x00: break;        // NOP
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
    break;

    // ---------------------- LXI -----------------------------  
  case 0x01:               // LXI B, word
    state->registers[1] = opcode[1]; // little endian load
    state->registers[0] = opcode[2];
    state->pc += 2; // advance PC 2 because next 2 bytes were data
    break;
  case 0x11:               // LXI D, word
    state->registers[3] = opcode[1];
    state->registers[2] = opcode[2];
    state->pc += 2;
    break;
  case 0x21:               // LXI H, word
    state->registers[5] = opcode[1];
    state->registers[4] = opcode[2];
    state->pc += 2;
    break;
    // --------------------- STAX -----------------------------
  case 0x02:               // STAX B, store A indirect
    break;
  
    // --------------------- LDAX -----------------------------

  case 0x0A:               // LDAX B, load A indirect
    { // DEFINE SCOPE LOCAL TO SWITCH CASE
      uint16_t BC = (state->registers[0]<<8) | (state->registers[1]);

      // value in memory at BC
      uint8_t mem = state->memory[BC];
      
      state->registers[7] = mem;
    }
    break;

  case 0x1A:               // LDAX D, load A indirect
    {
      uint16_t DE = (state->registers[2]<<8) | (state->registers[3]);
      
      // value in memory at BC
      uint8_t mem = state->memory[DE];
      
      state->registers[7] = mem;
    }
    break;

    // ----------------------- STA -----------------------------

  case 0x32:               // STA, store A direct
    {
      uint8_t addr = opcode[1];
      
      state->memory[addr] = state->registers[7]; // store A at addr
      
      state->pc += 1;
    }
    break;

    // ----------------------- LDA -----------------------------

  case 0x3A:               // LDA, load A direct
    {
      uint8_t addr = opcode[1];
      
      state->registers[7] = state->memory[addr]; // store value at addr into A
      
      state->pc += 1;
    }
    break;

  case 0xEB:               // XCHG, exchange DE HL
    {
      // H <-> D, L <-> E
      uint8_t temp = state->registers[4]; // H
      state->registers[4] = state->registers[2];
      state->registers[2] = temp;
      
      temp = state->registers[5]; // D
      state->registers[5] = state->registers[3];
      state->registers[3] = temp;
    }
    break;

    // -------------------- STACK OPS ---------------------------

    // --------------------- PUSH------------------------------
  case 0xC5:               // PUSH B, push register BC on stack
    // SP-1 <- RP1
    state->memory[state->sp - 1] = state->registers[0];
    // SP-2 <- RP2
    state->memory[state->sp - 2] = state->registers[1];
    // SP <- SP - 2
    state->sp -= 2;
    break;
  case 0xD5:               // PUSH D, push register DE on stack
    // SP-1 <- RP1
    state->memory[state->sp - 1] = state->registers[2];
    // SP-2 <- RP2
    state->memory[state->sp - 2] = state->registers[3];
    // SP <- SP - 2
    state->sp -= 2;
    break;
  case 0xE5:               // PUSH H, push register HL on stack
    // SP-1 <- RP1
    state->memory[state->sp - 1] = state->registers[4];
    // SP-2 <- RP2
    state->memory[state->sp - 2] = state->registers[5];
    // SP <- SP - 2
    state->sp -= 2;
    break;
  case 0xF5:                 // PUSH PSW, push A and flags on stack
    {
      // (sp - 2) <- flags,  s, z p, c, ac
      uint8_t flags = (state->cc->s << 7) | (state->cc->z << 6) | (state->cc->p << 5) | (state->cc->c << 4) | (state->cc->ac << 3) | (state->IE << 2);
      
      state->memory[state->sp - 2] = flags;
      
      // (sp - 1) <- A
      state->memory[state->sp - 1] = state->registers[7];
      
      // sp <- sp - 2
      state->sp -= 2;
    }
    break;
    
    // --------------------- POP --------------------------------
  case 0xC1:               // POP B, pops 16 bits off stack into BC
    // RP1 <- (SP) + 1
    state->registers[0] = state->memory[state->sp + 1];
    // RP2 <- (SP)
    state->registers[1] = state->memory[state->sp];

    state->sp += 2; // shift stack pointer back
    break;
  case 0xD1:               // POP D, pops 16 bits off stack into DE
    // RP1 <- (SP) + 1
    state->registers[2] = state->memory[state->sp + 1];
    // RP2 <- (SP)
    state->registers[3] = state->memory[state->sp];

    state->sp += 2; // shift stack pointer back
    break;
  case 0xE1:               // POP H, pops 16 bits off stack into HF
    // RP1 <- (SP) + 1
    state->registers[4] = state->memory[state->sp + 1];
    // RP2 <- (SP)
    state->registers[5] = state->memory[state->sp];

    state->sp += 2; // shift stack pointer back
    break;

  case 0xF1:               // POP PSW, pop a and flags off stack
    // restore flags, (flags) <- (sp), s, z p, c, ac
    state->cc->s = state->memory[state->sp] >> 7;
    state->cc->z = (state->memory[state->sp] & 0x40) >> 6;
    state->cc->p = (state->memory[state->sp] & 0x20) >> 5;
    state->cc->c = (state->memory[state->sp] & 0x10) >> 4;
    state->cc->ac = (state->memory[state->sp] & 0x08) >> 3;
    state->IE = (state->memory[state->sp] & 0x04) >> 2;

    // A <- (sp + 1)
    state->registers[7] = state->memory[state->sp + 1];

    // sp <- sp + 2
    state->sp += 2;
    break;

  case 0xF9:              // SPHL, HL to stack pointer
    // (SP) <- (H) : (L)
    state->sp = (state->registers[4] << 8) | (state->registers[5] & 0xff);
    break;

    // -------------------- JUMP -------------------------------
  case 0xC3:              // JMP, unconditional
    // PC <- ADDR
    {
      uint16_t addr = (opcode[1] << 8) | (opcode[2] & 0xff);
      state->pc = state->memory[addr];
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
      uint16_t addr = (state->registers[4] << 8) | (state->registers[5] && 0xff);
      state->pc = addr;
    }
    break;

    // ---------------------- CALL ------------------------------
  case 0xCD:               // CALL, call unconditional
    // (SP - 1) <- PCH, (SP - 2) <- PCL, sp += 2, PC <- addr
    {
      state->memory[state->sp - 1] = (uint8_t) (state->pc >> 8);
      state->memory[state->sp - 2] = (uint8_t) (state->pc && 0x00ff);

      state->sp += 2;
      
      uint16_t addr = (opcode[1] << 8) | (opcode[2] && 0xff);
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
      uint16_t updatedPC = (state->memory[state->sp] << 8) | (state->memory[state->sp + 1] && 0xff);
      
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
    break;

    // ---------------------- INX -------------------------------
  case 0x03:               // INX B, increment BC
    {
      uint16_t BC = (state->registers[0]<<8) | (state->registers[1]);
      
      uint32_t sum = BC++;
      // populate bc
      state->registers[0] = (uint8_t) (sum>>8);
      state->registers[1] = (uint8_t) (sum & 0x0f);
      // no flags affected
    }
    break;
  case 0x13:               // INX D, increment DE
    {
      uint16_t DE = (state->registers[2]<<8) | (state->registers[3]);
      
      uint32_t sum = DE++;
      // populate DE
      state->registers[2] = (uint8_t) (sum>>8);
      state->registers[3] = (uint8_t) (sum & 0x0f);
      // no flags affected
    }
    break;
  case 0x23:               // INX H, increment HL
    {
      uint16_t HL = (state->registers[4]<<8) | (state->registers[5]);
      
      uint32_t sum = HL++;
      // populate HL
      state->registers[4] = (uint8_t) (sum>>8);
      state->registers[5] = (uint8_t) (sum & 0x0f);
      // no flags affected
    }
    break;
      
    // ----------------------- DCX -----------------------------
  case 0x0B:               // DCX B, decrement BC
    {
      uint16_t BC = (state->registers[0]<<8) | (state->registers[1]);
      
      uint32_t sum = BC--;
      // populate bc
      state->registers[0] = (uint8_t) (sum>>8);
      state->registers[1] = (uint8_t) (sum & 0x0f);
      // no flags affected
    }
    break;
  case 0x1B:               // DCX D, decrement DE
    {
      uint16_t DE = (state->registers[2]<<8) | (state->registers[3]);
      
      uint32_t sum = DE--;
      // populate de
      state->registers[2] = (uint8_t) (sum>>8);
      state->registers[3] = (uint8_t) (sum & 0x0f);
      // no flags affected
    }
    break;
  case 0x2B:               // DCX H, decrement HL
    {
      uint16_t HL = (state->registers[4]<<8) | (state->registers[5]);
      
      uint32_t sum = HL--;
      // populate hl
      state->registers[4] = (uint8_t) (sum>>8);
      state->registers[5] = (uint8_t) (sum & 0x0f);
      // no flags affected
    }
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
    break;

    // ---------------------- ADI --------------------------------
  case 0xC6:               // ADI byte, add immediate
    {
      uint16_t sum = (uint16_t) state->registers[7] + (uint16_t) opcode[1]; // next byte
      
      UpdateCCSimple(state, sum);
      state->registers[7] = sum & 0xff;
      state->pc += 1; // for the immediate byte
    }
    break;

  case 0xC7:               // ACI, add immediate to A with carry
    {
      uint16_t sum = (uint16_t) state->registers[7] + (uint16_t) opcode[1] + (uint16_t) state->cc->c;
      
      UpdateCCSimple(state, sum);
      state->registers[7] = sum & 0xff;
      state->pc += 1;
    }
    break;

    // ------------------------ DAD ---------------------------
  case 0x09:              // DAD B, BC + HL --> HL, updates only carry flag
    {
      uint16_t BC = (state->registers[0]<<8) | (state->registers[1]);
      uint16_t HL = (state->registers[4]<<8) | (state->registers[5]);
      
      uint32_t sum = BC + HL;
      // populate HL
      state->registers[4] = (uint8_t) (sum>>8);
      state->registers[5] = (uint8_t) (sum & 0x0f);
      
      // update carry flag
      state->cc->c = (sum > 0xffff);
    }
    break;
  case 0x19:              // DAD D, DE + HL --> HL, updates only carry flag
    {
      uint16_t DE = (state->registers[2]<<8) | (state->registers[3]);
      uint16_t HL = (state->registers[4]<<8) | (state->registers[5]);
      
      uint32_t sum = DE + HL;
      // populate HL
      state->registers[4] = (uint8_t) (sum>>8);
      state->registers[5] = (uint8_t) (sum & 0x0f);
      
      // update carry flag
      state->cc->c = (sum > 0xffff);
    }
    break;
  case 0x29:              // DAD H, HL + HL --> HL, updates only carry flag
    {
      uint16_t HL = (state->registers[4]<<8) | (state->registers[5]);
      
      uint32_t sum = HL + HL;
      // populate HL
      state->registers[4] = (uint8_t) (sum>>8);
      state->registers[5] = (uint8_t) (sum & 0x0f);
      
      // update carry flag
      state->cc->c = (sum > 0xffff);
    }
    break;
  case 0x39:              // DAD SP, SP + HL --> HL, updates only carry flag
    {
      uint16_t HL = (state->registers[4]<<8) | (state->registers[5]);
      
      uint32_t sum = state->sp + HL;
      // populate HL
      state->registers[4] = (uint8_t) (sum>>8);
      state->registers[5] = (uint8_t) (sum & 0x0f);
      
      // update carry flag
      state->cc->c = (sum > 0xffff);
    }
    break;

    // ------------------- SUBTRACTS --------------------------

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
    break;

    // ----------------------- ANI ----------------------------
  case 0xE6:              // ANI, and immediate with A
    {
      uint8_t result = state->registers[7] & opcode[1];
      
      state->registers[7] = result;
      
      UpdateCCSimple(state, result);
      state->cc->c = 0; // carry is zeroed
      
      state->pc += 1;
    }
    break;

    // ------------------------ CPI ---------------------------
  case 0xFE:              // CPI, compare immediate with A
    // condition bits set by (A) - Data
    {
      uint8_t result = state->registers[7] - opcode[1];
      
      UpdateCCSimple(state, result);
      
      state->pc += 1;
    }
    break;
    
    // ------------------- ROTATE -----------------------------
  case 0x0F:              // RRC, rotate A right
    {
      uint8_t a0 = state->registers[7] & 0x01;

      // set carry = A0
      state->cc->c = a0;
      
      // rotate A right
      // rotate
      state->registers[7] >>= 1;
      
      // A7 <- A0
      state->registers[7] &= 0x7f;         // clear last bit
      state->registers[7] |= (a0 << 7);    // update last bit
    }
    break;

    // ------------------ INPUT/OUTPUT -------------------------
    // TODO: THESE ARE TEMPORARY PLACE HOLDERS FOR THESE FUNCTIONS
    // SHOULD BE MOVED OUTSIDE
  case 0xDB: ;            // IN, input
    // (A) <- input device
    // data byte = port
    {
      uint8_t port = opcode[1];

      state->registers[7] = (*state->drivers->in[port]) ();
      state->pc += 1;
    }
    break;
  case 0xD3:              // OUT, output
    // output device <- (A)
    {
      uint8_t port = opcode[1];

      (*state->drivers->out[port]) (state->registers[7]);
      state->pc += 1;
    }
    break;

    // --------------------- CONTROL -----------------------------
  case 0xFB:              // EL, enable interrupts
    state->IE = 1;
    break;
  case 0xF3:              // DI, disable interrupts
    state->IE = 0;
    break;
  case 0x76:              // HLT, halt
    printf ("Recieved halt");
    exit(0);
    break;
  default:
    UnimplementedInstruction(state);
  }
  
  state->pc += 1; // move up one for opcode itself

  //  PRINT CURRENT INSTRUCTION AND STATE
  printf("\t CURRENT INSTRUCTION: $%02x\n", *opcode);
  
  printf("\t CURRENT STATE: \n");
  
  printf("\tC=%d, P=%d, S=%d, Z = %d\n", state->cc->c, state->cc->p, state->cc->s, state->cc->z);
  printf("\tA $%02x B $%02x C $%02x D $%02x E $%02x H $%02x L $%02x SP %04x\n", state->registers[7], state->registers[0], state->registers[1], state->registers[2], state->registers[3], state->registers[4], state->registers[5], state->sp);

  printf("\n");
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
  state->memory = (uint8_t *) malloc (65535 * sizeof (uint8_t));

  state->cc = ConditionCodes_init();

  state->IE = 0;

  return state;
}

uint8_t empty_IN () {
  return 0;
}

void empty_OUT (uint8_t ac) {
  return;
}

Drivers_T Drivers_init () {
  Drivers_T drivers = (struct Drivers *) malloc (sizeof (struct Drivers));

  drivers->in[0] = empty_IN;
  drivers->in[1] = empty_IN;
  drivers->in[2] = empty_IN;
  drivers->in[3] = empty_IN;
  drivers->in[4] = empty_IN;
  drivers->in[5] = empty_IN;
  drivers->in[6] = empty_IN;
  drivers->in[7] = empty_IN;

  drivers->out[0] = empty_OUT;
  drivers->out[1] = empty_OUT;
  drivers->out[2] = empty_OUT;
  drivers->out[3] = empty_OUT;
  drivers->out[4] = empty_OUT;
  drivers->out[5] = empty_OUT;
  drivers->out[6] = empty_OUT;
  drivers->out[7] = empty_OUT;

  return drivers;
}

void State8080_load_mem(State8080_T state, int start, unsigned char *buffer) {
  for (int i = start; i < 2048; i++) {
    state->memory[i] = (uint8_t) *buffer;
    buffer++;
  }
}
