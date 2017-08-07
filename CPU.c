/*
  File name : CPU.c
  
  Description : contains the bulk of the information for the 8080 cpu
  Author : Antony Toron

 */

struct ConditionCodes { // specifying bit count (1 bit flags/flip flops)
  uint8_t z : 1;   // zero flag
  uint8_t s : 1;   // sign flag (0 = POSITIVE, 1 = NEGATIVE)
  uint8_t p : 1;   // parity flag (0 = ODD, 1 = EVEN)
  uint8_t c : 1;   // carry flag (0 = no carry, 1 = carry)
  uint8_t ac : 1;  // auxiliary carry (carry out of 3 bit accumulator - DAA) 
  uint8_t pad : 3; // ??
  // IE: flip flop for interrupt enabled
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
  uint8_t IE;          // interrupt enabled flip flop
};


void UnimplementedInstruction(State8080_T state) {
  // PC will have been incremented, so it should be decremented back

  fprintf (stderr, "Unimplemented instruction\n");
  exit(1);
}

uint8_t Parity(uint16_t target) {
  if (target % 2 == 0)
    return 1;
  return 0;
}

void UpdateCCSimple(State8080_T state, uint16_t result) {
  state->cc->z = ((result & 0xff) == 0); // check if 0, AND with all 1s
  state->cc->s = ((result & 0x80) != 0); // check if bit 7 is set
  state->cc->c = (result > 0xff);        // check if sum > 256
  state->cc->p = Parity(result & 0xff);  // check parity
}

void UpdateCCZeroSignParity(State8080_T state, uint16_t result) {
  state->cc->z = ((result & 0xff) == 0);
  state->cc->s = ((result & 0x80) != 0);
  state->cc->p = Parity(result & 0xff);
}

uint8_t GetRegister(State8080_T state, uint8_t reg) {
  switch (reg) {
  case 0x06:
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

uint8_t AddToRegister (State8080_T state, uint8_t reg, uint8_t add) {
  switch (reg) {
  case 0x06:
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

void MOV (State8080_T state) {
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

// ADD register r from opcode to accumulator (a)
void ADD_R (State8080_T state) {
  unsigned char *opcode = &state->memory[state->pc];
  uint8_t reg = (*opcode) & 0x07;
  uint8_t reg_value = GetRegister(state, reg);
  uint16_t sum = (uint16_t) state->registers[7] + (uint16_t) reg_value;
  
  UpdateCCSimple(state, sum);
  state->registers[7] = sum & 0xff;              // update accumulator
}

// ADD register r from opcode to accumulator (a) with carry
void ADC_R (State8080_T state) {
  unsigned char *opcode = &state->memory[state->pc];
  uint8_t reg = (*opcode) & 0x07;
  uint8_t reg_value = GetRegister(state, reg);
  uint16_t sum = (uint16_t) state->registers[7] + (uint16_t) reg_value + (uint16_t) state->cc->c;
  
  UpdateCCSimple(state, sum);
  state->registers[7] = sum & 0xff;              // update accumulator
}


// INR register r from opcode
void INR_R (State8080_T state) {
  unsigned char *opcode = &state->memory[state->pc];
  uint8_t reg = ((*opcode) & 0x38)>>3;

  // source for CC info: http://altairclone.com/downloads/manuals/8080%20Programmers%20Manual.pdf vii

  switch (reg) {
  case 0x06:
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

// DCR register r from opcode
void DCR_R (State8080_T state) {
  unsigned char *opcode = &state->memory[state->pc];
  uin8_t reg = ((*opcode) & 0x38)>>3;

  switch (reg) {
  case 0x06:
    // H = high order bits, L = lower order bits, so shift H 8 bits up
    uint16_t offset = (state->h<<8) | (state->l);
    state->memory[offset]--; UpdateCCZeroSignParity(state, state->memory[offset]); break;
  default:
    if (reg > 7) {
      fprintf(stderr, "Could not find this register in addtoreg");
      exit(1);
    }
    state->registers[reg]--;
    UpdateCCZeroSignParity(state, state->registers[reg]);
    break;
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
  case 0x76:               // MOV B, Memory
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

    state->pc += 1; // increment for immediate
    
  case 0x01:               // LXI B, word
    state->c = opcode[1];
    state->b = opcode[2];
    state->pc += 2; // advance PC 2 because next 2 bytes were data
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
    uint16_t BC = (state->registers[0]<<8) | (state->registers[1]);

    uint32_t sum = BC++;
    // populate bc
    state->registers[0] = (uint8_t) (sum>>8);
    state->registers[1] = (uint8_t) (sum & 0x0f);
    // no flags affected  
  case 0x13:               // INX D, increment DE
    uint16_t DE = (state->registers[2]<<8) | (state->registers[3]);

    uint32_t sum = DE++;
    // populate DE
    state->registers[2] = (uint8_t) (sum>>8);
    state->registers[3] = (uint8_t) (sum & 0x0f);
    // no flags affected
  case 0x23:               // INX H, increment HL
    uint16_t HL = (state->registers[4]<<8) | (state->registers[5]);

    uint32_t sum = HL++;
    // populate HL
    state->registers[4] = (uint8_t) (sum>>8);
    state->registers[5] = (uint8_t) (sum & 0x0f);
    // no flags affected

    // ----------------------- DCX -----------------------------
  case 0x0B:               // DCX B, decrement BC
    uint16_t BC = (state->registers[0]<<8) | (state->registers[1]);

    uint32_t sum = BC--;
    // populate bc
    state->registers[0] = (uint8_t) (sum>>8);
    state->registers[1] = (uint8_t) (sum & 0x0f);
    // no flags affected  
  case 0x1B:               // DCX D, decrement DE
    uint16_t DE = (state->registers[2]<<8) | (state->registers[3]);

    uint32_t sum = DE--;
    // populate de
    state->registers[2] = (uint8_t) (sum>>8);
    state->registers[3] = (uint8_t) (sum & 0x0f);
    // no flags affected
  case 0x2B:               // DCX H, decrement HL
    uint16_t HL = (state->registers[4]<<8) | (state->registers[5]);

    uint32_t sum = HL--;
    // populate hl
    state->registers[4] = (uint8_t) (sum>>8);
    state->registers[5] = (uint8_t) (sum & 0x0f);
    // no flags affected

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
    uint16_t sum = (uint16_t) state->registers[7] + (uint16_t) opcode[1]; // next byte

    UpdateCCSimple(state, sum);
    state->registers[7] = sum & 0xff;
    state->pc += 1; // for the immediate byte
    break;

  case 0xC7:               // ACI, add immediate to A with carry
    uint16_t sum = (uint16_t) state->registers[7] + (uint16_t) opcode[1] + (uint16_t) state->cc->c;

    UpdateCCSimple(state, sum);
    state->registers[7] = sum & 0xff;
    state->pc += 1;
    break;

    // ------------------------ DAD ---------------------------
  case 0x09:              // DAD B, BC + HL --> HL, updates only carry flag
    uint16_t BC = (state->registers[0]<<8) | (state->registers[1]);
    uint16_t HL = (state->registers[4]<<8) | (state->registers[5]);

    uint32_t sum = BC + HL;
    // populate HL
    state->registers[4] = (uint8_t) (sum>>8);
    state->registers[5] = (uint8_t) (sum & 0x0f);

    // update carry flag
    state->cc->c = (sum > 0xffff);
    break;
  case 0x19:              // DAD D, DE + HL --> HL, updates only carry flag
    uint16_t DE = (state->registers[2]<<8) | (state->registers[3]);
    uint16_t HL = (state->registers[4]<<8) | (state->registers[5]);

    uint32_t sum = DE + HL;
    // populate HL
    state->registers[4] = (uint8_t) (sum>>8);
    state->registers[5] = (uint8_t) (sum & 0x0f);

    // update carry flag
    state->cc->c = (sum > 0xffff);
    break;
  case 0x29:              // DAD H, HL + HL --> HL, updates only carry flag
    uint16_t HL = (state->registers[4]<<8) | (state->registers[5]);

    uint32_t sum = HL + HL;
    // populate HL
    state->registers[4] = (uint8_t) (sum>>8);
    state->registers[5] = (uint8_t) (sum & 0x0f);

    // update carry flag
    state->cc->c = (sum > 0xffff);
    break;
  case 0x39:              // DAD SP, SP + HL --> HL, updates only carry flag
    uint16_t HL = (state->registers[4]<<8) | (state->registers[5]);

    uint32_t sum = state->sp + HL;
    // populate HL
    state->registers[4] = (uint8_t) (sum>>8);
    state->registers[5] = (uint8_t) (sum & 0x0f);

    // update carry flag
    state->cc->c = (sum > 0xffff);
    break;

    // ------------------- SUBTRACTS --------------------------
    
    
    
  }
  state->pc += 1; // move up one for opcode itself
}
