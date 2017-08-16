[1mdiff --git a/8080Arcade.c b/8080Arcade.c[m
[1mindex 476d56e..585ca91 100644[m
[1m--- a/8080Arcade.c[m
[1m+++ b/8080Arcade.c[m
[36m@@ -7,47 +7,18 @@[m
 [m
 #include "CPU.h"[m
 #include "Utils.h"[m
[31m-#include "Drivers.h"[m
 #include <stdio.h>[m
 #include <string.h>[m
 #include <stdlib.h>[m
 #include <stdint.h>[m
 [m
 // implement drivers[m
[31m-Drivers_T DefaultDrivers () {[m
[32m+[m[32mDrivers_T ArcadeDrivers () {[m
   Drivers_T drivers = Drivers_init();[m
 [m
   return drivers;[m
 }[m
 [m
[31m-// -------------------------- ARCADE CONFIGS --------------------------[m
[31m-[m
[31m-static ArcadeMachinePorts_T am_ports;[m
[31m-[m
[31m-void ArcadeOut4 (uint8_t ac) {[m
[31m-  Arcade8080_write4 (ac, am_ports);[m
[31m-}[m
[31m-[m
[31m-void ArcadeOut2 (uint8_t ac) {[m
[31m-  Arcade8080_write2 (ac, am_ports);[m
[31m-}[m
[31m-[m
[31m-uint8_t ArcadeRead3 () {[m
[31m-  return Arcade8080_read3(am_ports);[m
[31m-}[m
[31m-[m
[31m-Drivers_T ArcadeDrivers() {[m
[31m-  Drivers_T drivers = Drivers_init();[m
[31m-  am_ports = am_ports_init();[m
[31m-[m
[31m-  // shift registers[m
[31m-  config_drivers_out_port(drivers, &ArcadeOut4, 4);[m
[31m-  config_drivers_out_port(drivers, &ArcadeOut2, 2);[m
[31m-  config_drivers_in_port(drivers, &ArcadeRead3, 3);[m
[31m-  [m
[31m-  return drivers;[m
[31m-}[m
[31m-[m
 // -------------------------- INVADERS STATE ----------------------------[m
 [m
 void LOAD_ROM_invaders (State8080_T state) {[m
[36m@@ -63,8 +34,6 @@[m [mvoid LOAD_ROM_invaders (State8080_T state) {[m
 [m
 State8080_T INIT_STATE_invaders () {[m
   State8080_T state = State8080_init ();[m
[31m-[m
[31m-  State8080_config_drivers_default(state, ArcadeDrivers());[m
   [m
   return state;[m
 }[m
[36m@@ -77,7 +46,7 @@[m [mint main(int argc, char **argv) {[m
   if (strcmp(argv[1], "invaders") == 0) {[m
     state = INIT_STATE_invaders ();[m
 [m
[31m-    LOAD_ROM_invaders (state);[m
[32m+[m[32m    LOAD_ROM_invaders (state);[m[41m                          [m
   }[m
   else {[m
     fprintf (stderr, "This ROM is not supported");[m
[36m@@ -87,6 +56,6 @@[m [mint main(int argc, char **argv) {[m
   // BEGIN EMULATION[m
   while (1) {[m
     [m
[31m-    Emulate8080Op(state);[m
[32m+[m[32m    Emulate8080State(state);[m
   }[m
 }[m
[1mdiff --git a/8080Arcade.c~ b/8080Arcade.c~[m
[1mnew file mode 100644[m
[1mindex 0000000..eeeb3f1[m
[1m--- /dev/null[m
[1m+++ b/8080Arcade.c~[m
[36m@@ -0,0 +1,63 @@[m
[32m+[m[32m/*[m
[32m+[m[32m  File name : 8080Arcade.c[m
[32m+[m
[32m+[m[32m  Description : Using the intel8080 processor as arcade machine[m
[32m+[m[32m  Author : Antony Toron[m
[32m+[m[32m */[m
[32m+[m
[32m+[m[32m#include "CPU.h"[m
[32m+[m[32m#include "Utils.h"[m
[32m+[m[32m#include <stdio.h>[m
[32m+[m[32m#include <string.h>[m
[32m+[m[32m#include <stdlib.h>[m
[32m+[m
[32m+[m[32m// implement drivers[m
[32m+[m[32mDrivers_T ArcadeDrivers () {[m
[32m+[m[32m  Drivers_T drivers = Drivers_init();[m
[32m+[m
[32m+[m[32m  return drivers;[m
[32m+[m[32m}[m
[32m+[m
[32m+[m[32m// -------------------------- INVADERS STATE ----------------------------[m
[32m+[m
[32m+[m[32mvoid LOAD_ROM_invaders (State8080_T state) {[m
[32m+[m[32m  // load into memory[m
[32m+[m[32m  unsigned char *buffer = readFileIntoBuffer("invaders");[m
[32m+[m
[32m+[m[32m  // load the rom[m
[32m+[m[32m  for (int i = 0; i < 2048; i++) {[m
[32m+[m[32m    state->memory[i] = (uint8_t) *buffer;[m
[32m+[m[32m    buffer++;[m
[32m+[m[32m  }[m
[32m+[m[41m  [m
[32m+[m[32m  printf ("Successfully loaded ROM");[m
[32m+[m[41m  [m
[32m+[m[32m}[m
[32m+[m
[32m+[m[32mState8080_T INIT_STATE_invaders () {[m
[32m+[m[32m  State8080_T state = State8080_init ();[m
[32m+[m[41m  [m
[32m+[m[32m  return state;[m
[32m+[m[32m}[m
[32m+[m
[32m+[m[32m//------------------------------------------------------------------------[m
[32m+[m
[32m+[m[32mint main(int argc, char **argv) {[m
[32m+[m[32m  State8080_T state;[m
[32m+[m[41m  [m
[32m+[m[32m  if (strcmp(argv[1], "invaders") == 0) {[m
[32m+[m[32m    state = INIT_STATE_invaders ();[m
[32m+[m
[32m+[m[32m    LOAD_ROM_invaders (state);[m[41m                          [m
[32m+[m[32m  }[m
[32m+[m[32m  else {[m
[32m+[m[32m    fprintf (stderr, "This ROM is not supported");[m
[32m+[m[32m    exit(1);[m
[32m+[m[32m  }[m
[32m+[m
[32m+[m[32m  // BEGIN EMULATION[m
[32m+[m[32m  while (1) {[m
[32m+[m[41m    [m
[32m+[m[32m    Emulate8080Op(state);[m
[32m+[m[32m  }[m
[32m+[m[32m}[m
[1mdiff --git a/8080Arcade.o b/8080Arcade.o[m
[1mnew file mode 100644[m
[1mindex 0000000..231f1fc[m
Binary files /dev/null and b/8080Arcade.o differ
[1mdiff --git a/8080Notes~ b/8080Notes~[m
[1mnew file mode 100644[m
[1mindex 0000000..703655c[m
[1m--- /dev/null[m
[1m+++ b/8080Notes~[m
[36m@@ -0,0 +1,34 @@[m
[32m+[m[32mSpace Invaders, (C) Taito 1978, Midway 1979[m
[32m+[m
[32m+[m[32m   CPU: Intel 8080 @ 2MHz (CPU similar to the (newer) Zilog Z80)[m[41m    [m
[32m+[m
[32m+[m[32m   Interrupts: $cf (RST 8) at the start of vblank, $d7 (RST $10) at the end of vblank.[m[41m    [m
[32m+[m
[32m+[m[32m   Video: 256(x)*224(y) @ 60Hz, vertical monitor. Colours are simulated with a[m[41m    [m
[32m+[m[32m   plastic transparent overlay and a background picture.[m[41m    [m
[32m+[m[32m   Video hardware is very simple: 7168 bytes 1bpp bitmap (32 bytes per scanline).[m[41m    [m
[32m+[m
[32m+[m[32m   Sound: SN76477 and samples.[m[41m    [m
[32m+[m
[32m+[m[32m   Memory map:[m[41m    [m
[32m+[m[32m    ROM[m[41m    [m
[32m+[m[32m    $0000-$07ff:    invaders.h[m[41m    [m
[32m+[m[32m    $0800-$0fff:    invaders.g[m[41m    [m
[32m+[m[32m    $1000-$17ff:    invaders.f[m[41m    [m
[32m+[m[32m    $1800-$1fff:    invaders.e[m[41m    [m
[32m+[m
[32m+[m[32m    RAM[m[41m    [m
[32m+[m[32m    $2000-$23ff:    work RAM[m[41m    [m
[32m+[m[32m    $2400-$3fff:    video RAM[m[41m    [m
[32m+[m
[32m+[m[32m    $4000-:     RAM mirror[m
[32m+[m
[32m+[m
[32m+[m
[32m+[m[32mThe 8080 allows for 8 bytes for each ISR (interrupt handler), but space[m
[32m+[m[32minvaders handles this by jumping to a place with  more memory[m
[32m+[m
[32m+[m[32m- so, the ISR codes for interrupts start at $0, $8, $20, .., $38[m
[32m+[m[32m- notice that ISR 2 is  longer than 8 bytes, so probably it didnt expect[m
[32m+[m[32m  to ever need interrupt 3 since it overlaps onto it[m
[32m+[m
[1mdiff --git a/CPU.c b/CPU.c[m
[1mindex 0a30f08..388c333 100644[m
[1m--- a/CPU.c[m
[1m+++ b/CPU.c[m
[36m@@ -21,40 +21,6 @@[m [mstruct ConditionCodes { // specifying bit count (1 bit flags/flip flops)[m
   // IE: flip flop for interrupt enabled[m
 };[m
 [m
[31m-// PORT INFORMATION FOR 8080[m
[31m-[m
[31m-/* Ports:     */[m
[31m-/*     Read 1     */[m
[31m-/*     BIT 0   coin (0 when active)     */[m
[31m-/*         1   P2 start button     */[m
[31m-/*         2   P1 start button     */[m
[31m-/*         3   ?     */[m
[31m-/*         4   P1 shoot button     */[m
[31m-/*         5   P1 joystick left     */[m
[31m-/*         6   P1 joystick right     */[m
[31m-/*         7   ? */[m
[31m-[m
[31m-/*     Read 2     */[m
[31m-/*     BIT 0,1 dipswitch number of lives (0:3,1:4,2:5,3:6)     */[m
[31m-/*         2   tilt 'button'     */[m
[31m-/*         3   dipswitch bonus life at 1:1000,0:1500     */[m
[31m-/*         4   P2 shoot button     */[m
[31m-/*         5   P2 joystick left     */[m
[31m-/*         6   P2 joystick right     */[m
[31m-/*         7   dipswitch coin info 1:off,0:on     */[m
[31m-[m
[31m-/*     Read 3      shift register result     */[m
[31m-[m
[31m-/*     Write 2     shift register result offset (bits 0,1,2)     */[m
[31m-/*     Write 3     sound related     */[m
[31m-/*     Write 4     fill shift register     */[m
[31m-/*     Write 5     sound related     */[m
[31m-/*     Write 6     strange 'debug' port? eg. it writes to this port when     */[m
[31m-/*             it writes text to the screen (0=a,1=b,2=c, etc)     */[m
[31m-[m
[31m-/*     (write ports 3,5,6 can be left unemulated, read port 1=$01 and 2=$00     */[m
[31m-/*     will make the game run, but but only in attract mode)  */ [m
[31m-[m
 // describe the behavior for each respective port[m
 // in should return a value to read into accumulator[m
 // out takes value to put into output[m
[36m@@ -85,14 +51,14 @@[m [mstruct State8080 {[m
   ConditionCodes_T cc;[m
   Drivers_T drivers;[m
   uint8_t IE : 1;          // interrupt enabled flip flop[m
[32m+[m[32m  uint8_t *interrupts; // interrupt queue[m
 };[m
 [m
 [m
 static void UnimplementedInstruction(State8080_T state) {[m
   // PC will have been incremented, so it should be decremented back[m
[31m-  unsigned char *opcode = &state->memory[state->pc];[m
[31m-  [m
[31m-  fprintf (stderr, "Unimplemented instruction : $%02x\n", *opcode);[m
[32m+[m
[32m+[m[32m  fprintf (stderr, "Unimplemented instruction\n");[m
   exit(1);[m
 }[m
 [m
[36m@@ -154,15 +120,15 @@[m [mstatic void MOV (State8080_T state) {[m
 [m
   if (dest == 6 || source == 6) {[m
     if (dest == 6 && source != 6) {[m
[31m-      uint16_t offset = (state->registers[4]<<8) | (state->registers[5] & 0xff);[m
[32m+[m[32m      uint16_t offset = (state->registers[4]<<8) | (state->registers[5]);[m
       state->memory[offset] = state->registers[source];[m
     }[m
     else if (dest != 6 && source == 6) {[m
[31m-      uint16_t offset = (state->registers[4]<<8) | (state->registers[5] & 0xff);[m
[31m-      state->registers[dest] = state->memory[offset];[m
[32m+[m[32m      uint16_t offset = (state->registers[4]<<8) | (state->registers[5]);[m
[32m+[m[32m      state->registers[source] = state->memory[offset];[m
     }[m
     else {[m
[31m-      uint16_t offset = (state->registers[4]<<8) | (state->registers[5] & 0xff);[m
[32m+[m[32m      uint16_t offset = (state->registers[4]<<8) | (state->registers[5]);[m
       state->memory[offset] = state->memory[offset];[m
     }[m
   }[m
[36m@@ -176,7 +142,7 @@[m [mstatic void MVI (State8080_T state) {[m
   uint8_t dest = ((*opcode) & 0x38)>>3;[m
 [m
   if (dest == 6) {[m
[31m-    uint16_t offset = (state->registers[4]<<8) | (state->registers[5] & 0xff);[m
[32m+[m[32m    uint16_t offset = (state->registers[4]<<8) | (state->registers[5]);[m
     state->memory[offset] = opcode[1];[m
   }[m
   else {[m
[36m@@ -295,18 +261,18 @@[m [mstatic void DCR_R (State8080_T state) {[m
 static void JMP_conditional (State8080_T state, uint8_t condition) {[m
    unsigned char *opcode = &state->memory[state->pc];[m
    if (condition) {[m
[31m-     uint16_t addr = (opcode[2] << 8) | (opcode[1] & 0xff); // little-endian[m
[31m-     state->pc = addr;[m
[32m+[m[32m     uint16_t addr = (opcode[1] << 8) | (opcode[2] & 0xff);[m
[32m+[m[32m     state->pc = state->memory[addr];[m
    }[m
    else[m
[31m-     state->pc += 3;[m
[32m+[m[32m     state->pc += 2;[m
 }[m
 [m
 static void RET_conditional (State8080_T state, uint8_t condition) {[m
   unsigned char *opcode = &state->memory[state->pc];[m
   if (condition) {[m
     // PCL(low order bits) <- sp, PCH <- sp + 1, SP <- sp + 2[m
[31m-    uint16_t updatedPC = (state->memory[state->sp + 1] << 8) | (state->memory[state->sp] & 0xff);[m
[32m+[m[32m    uint16_t updatedPC = (state->memory[state->sp] << 8) | (state->memory[state->sp + 1] && 0xff);[m
     [m
     state->pc = updatedPC;[m
     state->sp += 2;[m
[36m@@ -315,22 +281,31 @@[m [mstatic void RET_conditional (State8080_T state, uint8_t condition) {[m
 [m
 static void CALL_conditional (State8080_T state, uint8_t condition) {[m
   unsigned char *opcode = &state->memory[state->pc];[m
[31m-[m
[31m-  state->pc += 1; // for op[m
   if (condition) {[m
[31m-    // (sp - 1) <- PCH, (sp - 2) <- PCL, SP += 2, PC <- addr[m
[31m-    state->sp += 2; // for data bytes[m
[31m-    [m
[32m+[m[32m    // (sp - 1) <- PCH, (sp - 2) <- PCL, SP += 2, PC <- addr[m[41m [m
     state->memory[state->sp - 1] = (state->pc >> 8) & 0xff;[m
     state->memory[state->sp - 2] = (uint8_t) (state->pc & 0x00ff);[m
 [m
[31m-    [m
[31m-    state->pc = (opcode[2] << 8) | (opcode[1] & 0xff); // little-endian[m
[32m+[m[32m    state->sp += 2;[m
[32m+[m[32m    state->pc = (opcode[1] << 8) | (opcode[2] & 0xff); // should this be reversed?[m
   }[m
   else[m
     state->pc += 2; // for data bytes[m
 }[m
 [m
[32m+[m[32mvoid PUSH_PSW(State8080_T state) {[m
[32m+[m[32m  // (sp - 2) <- flags,  s, z p, c, ac[m
[32m+[m[32m  uint8_t flags = (state->cc->s << 7) | (state->cc->z << 6) | (state->cc->p << 5) | (state->cc->c << 4) | (state->cc->ac << 3) | (state->IE << 2);[m
[32m+[m[41m  [m
[32m+[m[32m  state->memory[state->sp - 2] = flags;[m
[32m+[m[41m  [m
[32m+[m[32m  // (sp - 1) <- A[m
[32m+[m[32m  state->memory[state->sp - 1] = state->registers[7];[m
[32m+[m[41m  [m
[32m+[m[32m  // sp <- sp - 2[m
[32m+[m[32m  state->sp -= 2;[m
[32m+[m[32m}[m
[32m+[m
 uint8_t MachineIN (State8080_T state, uint8_t port) {[m
   return 0; // placeholder[m
 }[m
[36m@@ -339,20 +314,24 @@[m [mvoid MachineOUT (State8080_T state, uint8_t port) {[m
   return; // placeholder[m
 }[m
 [m
[31m-void Emulate8080Op(State8080_T state) {[m
[31m-  unsigned char *opcode;[m
[31m-  [m
[31m-  // get opcode from memory at PC, PC steps away from start of memory[m
[31m-  opcode = &state->memory[state->pc];[m
[31m-[m
[31m-  printf("Executing $%02x", *opcode);[m
[32m+[m[32m// similar to call, but calls interrupt code[m
[32m+[m[32mvoid RST (State8080_T state) {[m
[32m+[m[32m  unsigned char *opcode = &state->memory[state->pc];[m
   [m
[31m-  // NOTE: many of the instructions update PC <- PC + 1, for the op[m
[32m+[m[32m  // push program counter[m
[32m+[m[32m  state->memory[state->sp - 1] = (uint8_t) (state->pc >> 8);[m
[32m+[m[32m  state->memory[state->sp - 2] = (uint8_t) (state->pc && 0x00ff);[m
   [m
[32m+[m[32m  state->sp += 2;[m
[32m+[m
[32m+[m[32m  // 11AAA111 -> 00AAA000[m
[32m+[m[32m  uint16_t addr = (opcode[0] & 0x38) & 0xffff;[m
[32m+[m[32m  state->pc = addr;[m
[32m+[m[32m}[m
[32m+[m
[32m+[m[32mvoid Emulate8080Op(State8080_T state, unsigned char *opcode) {[m[41m  [m
   switch (*opcode) {[m
[31m-  case 0x00:               // NOP[m
[31m-    state->pc += 1;[m
[31m-    break;        [m
[32m+[m[32m  case 0x00: break;        // NOP[m
     // --------------- MOVE/LOADS/STORES ----------------------[m
     [m
     // TODO: FORMAT THESE COMMENTS FOR MOV PROPERLY[m
[36m@@ -422,7 +401,6 @@[m [mvoid Emulate8080Op(State8080_T state) {[m
   case 0x7E:               // MOV B, D[m
   case 0x7F:               // MOV B, E[m
     MOV (state);[m
[31m-    state->pc += 1;[m
     // no cc affected[m
     break;[m
 [m
[36m@@ -437,7 +415,6 @@[m [mvoid Emulate8080Op(State8080_T state) {[m
   case 0x3E:               // MVI A, move immediate to accumulator[m
     MVI (state);[m
     state->pc += 1; // increment for immediate[m
[31m-    state->pc += 1;[m
     break;[m
 [m
     // ---------------------- LXI -----------------------------  [m
[36m@@ -445,75 +422,67 @@[m [mvoid Emulate8080Op(State8080_T state) {[m
     state->registers[1] = opcode[1]; // little endian load[m
     state->registers[0] = opcode[2];[m
     state->pc += 2; // advance PC 2 because next 2 bytes were data[m
[31m-    state->pc += 1;[m
     break;[m
   case 0x11:               // LXI D, word[m
     state->registers[3] = opcode[1];[m
     state->registers[2] = opcode[2];[m
     state->pc += 2;[m
[31m-    state->pc += 1;[m
     break;[m
   case 0x21:               // LXI H, word[m
     state->registers[5] = opcode[1];[m
     state->registers[4] = opcode[2];[m
     state->pc += 2;[m
[31m-    state->pc += 1;[m
     break;[m
     // --------------------- STAX -----------------------------[m
   case 0x02:               // STAX B, store A indirect[m
[31m-    state->pc += 1;[m
     break;[m
   [m
     // --------------------- LDAX -----------------------------[m
 [m
   case 0x0A:               // LDAX B, load A indirect[m
     { // DEFINE SCOPE LOCAL TO SWITCH CASE[m
[31m-      uint16_t BC = (state->registers[0]<<8) | (state->registers[1] & 0xff);[m
[32m+[m[32m      uint16_t BC = (state->registers[0]<<8) | (state->registers[1]);[m
 [m
       // value in memory at BC[m
       uint8_t mem = state->memory[BC];[m
       [m
       state->registers[7] = mem;[m
     }[m
[31m-    state->pc += 1;[m
     break;[m
 [m
   case 0x1A:               // LDAX D, load A indirect[m
     {[m
[31m-      uint16_t DE = (state->registers[2]<<8) | (state->registers[3] & 0xff);[m
[32m+[m[32m      uint16_t DE = (state->registers[2]<<8) | (state->registers[3]);[m
       [m
[31m-      // value in memory at DE[m
[32m+[m[32m      // value in memory at BC[m
       uint8_t mem = state->memory[DE];[m
       [m
       state->registers[7] = mem;[m
     }[m
[31m-    state->pc += 1;[m
     break;[m
 [m
     // ----------------------- STA -----------------------------[m
 [m
   case 0x32:               // STA, store A direct[m
     {[m
[31m-      uint16_t addr = (opcode[2]<<8) | (opcode[1] & 0xff);[m
[32m+[m[32m      uint8_t addr = opcode[1];[m
       [m
       state->memory[addr] = state->registers[7]; // store A at addr[m
       [m
[31m-      state->pc += 2; // for data bytes[m
[32m+[m[32m      state->pc += 1;[m
     }[m
[31m-    state->pc += 1;[m
     break;[m
 [m
     // ----------------------- LDA -----------------------------[m
 [m
   case 0x3A:               // LDA, load A direct[m
[31m-    { // opcode[1] -> low_addr, opcode[2] -> high_addr[m
[31m-      uint16_t addr = (opcode[2]<<8) | (opcode[1] & 0xff);[m
[32m+[m[32m    {[m
[32m+[m[32m      uint8_t addr = opcode[1];[m
       [m
       state->registers[7] = state->memory[addr]; // store value at addr into A[m
       [m
[31m-      state->pc += 2; // for data bytes[m
[32m+[m[32m      state->pc += 1;[m
     }[m
[31m-    state->pc += 1;[m
     break;[m
 [m
   case 0xEB:               // XCHG, exchange DE HL[m
[36m@@ -527,26 +496,20 @@[m [mvoid Emulate8080Op(State8080_T state) {[m
       state->registers[5] = state->registers[3];[m
       state->registers[3] = temp;[m
     }[m
[31m-    state->pc += 1;[m
     break;[m
 [m
     // -------------------- STACK OPS ---------------------------[m
 [m
     // --------------------- PUSH------------------------------[m
   case 0xC5:               // PUSH B, push register BC on stack[m
[31m-    state->pc += 1; // for op[m
[31m-[m
     // SP-1 <- RP1[m
     state->memory[state->sp - 1] = state->registers[0];[m
     // SP-2 <- RP2[m
     state->memory[state->sp - 2] = state->registers[1];[m
     // SP <- SP - 2[m
     state->sp -= 2;[m
[31m-    [m
     break;[m
   case 0xD5:               // PUSH D, push register DE on stack[m
[31m-    state->pc += 1; // for op[m
[31m-    [m
     // SP-1 <- RP1[m
     state->memory[state->sp - 1] = state->registers[2];[m
     // SP-2 <- RP2[m
[36m@@ -555,8 +518,6 @@[m [mvoid Emulate8080Op(State8080_T state) {[m
     state->sp -= 2;[m
     break;[m
   case 0xE5:               // PUSH H, push register HL on stack[m
[31m-    state->pc += 1; // for op[m
[31m-[m
     // SP-1 <- RP1[m
     state->memory[state->sp - 1] = state->registers[4];[m
     // SP-2 <- RP2[m
[36m@@ -566,59 +527,37 @@[m [mvoid Emulate8080Op(State8080_T state) {[m
     break;[m
   case 0xF5:                 // PUSH PSW, push A and flags on stack[m
     {[m
[31m-      state->pc += 1; // for op[m
[31m-[m
[31m-      // (sp - 2) <- flags,  s, z p, c, ac[m
[31m-      uint8_t flags = (state->cc->s << 7) | (state->cc->z << 6) | (state->cc->p << 5) | (state->cc->c << 4) | (state->cc->ac << 3) | (state->IE << 2);[m
[31m-      [m
[31m-      state->memory[state->sp - 2] = flags;[m
[31m-      [m
[31m-      // (sp - 1) <- A[m
[31m-      state->memory[state->sp - 1] = state->registers[7];[m
[31m-      [m
[31m-      // sp <- sp - 2[m
[31m-      state->sp -= 2;[m
[32m+[m[32m      PUSH_PSW(state);[m
     }[m
     break;[m
     [m
     // --------------------- POP --------------------------------[m
   case 0xC1:               // POP B, pops 16 bits off stack into BC[m
[31m-    state->pc += 1; // for op[m
[31m-[m
     // RP1 <- (SP) + 1[m
     state->registers[0] = state->memory[state->sp + 1];[m
     // RP2 <- (SP)[m
     state->registers[1] = state->memory[state->sp];[m
 [m
     state->sp += 2; // shift stack pointer back[m
[31m-[m
     break;[m
   case 0xD1:               // POP D, pops 16 bits off stack into DE[m
[31m-    state->pc += 1; // for op[m
[31m-    [m
     // RP1 <- (SP) + 1[m
     state->registers[2] = state->memory[state->sp + 1];[m
     // RP2 <- (SP)[m
     state->registers[3] = state->memory[state->sp];[m
 [m
     state->sp += 2; // shift stack pointer back[m
[31m-[m
     break;[m
   case 0xE1:               // POP H, pops 16 bits off stack into HF[m
[31m-    state->pc += 1; // for op[m
[31m-[m
     // RP1 <- (SP) + 1[m
     state->registers[4] = state->memory[state->sp + 1];[m
     // RP2 <- (SP)[m
     state->registers[5] = state->memory[state->sp];[m
 [m
     state->sp += 2; // shift stack pointer back[m
[31m-[m
     break;[m
 [m
   case 0xF1:               // POP PSW, pop a and flags off stack[m
[31m-    state->pc += 1; // for op[m
[31m-[m
     // restore flags, (flags) <- (sp), s, z p, c, ac[m
     state->cc->s = state->memory[state->sp] >> 7;[m
     state->cc->z = (state->memory[state->sp] & 0x40) >> 6;[m
[36m@@ -632,33 +571,19 @@[m [mvoid Emulate8080Op(State8080_T state) {[m
 [m
     // sp <- sp + 2[m
     state->sp += 2;[m
[31m-[m
     break;[m
 [m
   case 0xF9:              // SPHL, HL to stack pointer[m
[31m-    state->pc += 1; // for op[m
[31m-[m
     // (SP) <- (H) : (L)[m
     state->sp = (state->registers[4] << 8) | (state->registers[5] & 0xff);[m
     break;[m
[31m-  case 0x31:              // LXI SP, load immediate stack pointer[m
[31m-    {[m
[31m-      uint16_t addr = (opcode[2] << 8) | (opcode[1] & 0xff); // little-endian[m
[31m-      state->sp = addr;[m
[31m-    }[m
[31m-    state->pc += 2;[m
[31m-    state->pc += 1;[m
[31m-    break;[m
[31m-    [m
[32m+[m
     // -------------------- JUMP -------------------------------[m
   case 0xC3:              // JMP, unconditional[m
     // PC <- ADDR[m
[31m-    { // 11000011[low_addr][high_addr], where low_addr = ls 8 bits of mem addr[m
[31m-      state->pc += 1; // for op[m
[31m-[m
[31m-      // LITTLE ENDIAN[m
[31m-      uint16_t addr = (opcode[2] << 8) | (opcode[1] & 0xff);[m
[31m-      state->pc = addr;[m
[32m+[m[32m    {[m
[32m+[m[32m      uint16_t addr = (opcode[1] << 8) | (opcode[2] & 0xff);[m
[32m+[m[32m      state->pc = state->memory[addr];[m
     }[m
     break;[m
   case 0xDA:              // JC, jump on carry[m
[36m@@ -688,25 +613,21 @@[m [mvoid Emulate8080Op(State8080_T state) {[m
   case 0xE9:              // PCHL, HL to program counter[m
     // PC <- HL[m
     {[m
[31m-      uint16_t addr = (state->registers[4] << 8) | (state->registers[5] & 0xff);[m
[32m+[m[32m      uint16_t addr = (state->registers[4] << 8) | (state->registers[5] && 0xff);[m
       state->pc = addr;[m
     }[m
     break;[m
 [m
     // ---------------------- CALL ------------------------------[m
   case 0xCD:               // CALL, call unconditional[m
[31m-    // (SP - 1) <- PCH, (SP - 2) <- PCL, sp -= 2, PC <- addr[m
[31m-    // source = http://www.nj7p.org/Manuals/PDFs/Intel/9800153B.pdf[m
[32m+[m[32m    // (SP - 1) <- PCH, (SP - 2) <- PCL, sp += 2, PC <- addr[m
     {[m
[31m-      // for op, and two data bytes, next instruction IMPORTANT[m
[31m-      state->pc += 3;[m
[31m-      [m
[31m-      state->memory[state->sp - 1] = (uint8_t) ((state->pc >> 8) & 0xff);[m
[31m-      state->memory[state->sp - 2] = (uint8_t) (state->pc & 0x00ff);[m
[32m+[m[32m      state->memory[state->sp - 1] = (uint8_t) (state->pc >> 8);[m
[32m+[m[32m      state->memory[state->sp - 2] = (uint8_t) (state->pc && 0x00ff);[m
 [m
[31m-      state->sp -= 2;[m
[32m+[m[32m      state->sp += 2;[m
       [m
[31m-      uint16_t addr = (opcode[2] << 8) | (opcode[1] & 0xff); // little-endian[m
[32m+[m[32m      uint16_t addr = (opcode[1] << 8) | (opcode[2] && 0xff);[m
       state->pc = addr;[m
     }[m
     break;[m
[36m@@ -741,7 +662,7 @@[m [mvoid Emulate8080Op(State8080_T state) {[m
   case 0xC9:               // RET, return[m
     // PCL(low order bits) <- sp, PCH <- sp + 1, SP <- sp + 2[m
     {[m
[31m-      uint16_t updatedPC = (state->memory[state->sp + 1] << 8) | (state->memory[state->sp] & 0xff);[m
[32m+[m[32m      uint16_t updatedPC = (state->memory[state->sp] << 8) | (state->memory[state->sp + 1] && 0xff);[m
       [m
       state->pc = updatedPC;[m
       state->sp += 2;[m
[36m@@ -771,7 +692,19 @@[m [mvoid Emulate8080Op(State8080_T state) {[m
   case 0xE8:               // RPE, return on parity even[m
     RET_conditional (state, state->cc->p);[m
     break;[m
[31m-    [m
[32m+[m
[32m+[m[32m    // ---------------------- RESTART ---------------------------[m
[32m+[m
[32m+[m[32m  case 0xC7:               // RST 0[m
[32m+[m[32m  case 0xCF:               // RST 1[m
[32m+[m[32m  case 0xD7:               // RST 2[m
[32m+[m[32m  case 0xDF:               // RST 3[m
[32m+[m[32m  case 0xE7:               // RST 4[m
[32m+[m[32m  case 0xEF:               // RST 5[m
[32m+[m[32m  case 0xF7:               // RST 6[m
[32m+[m[32m  case 0xFF:               // RST 7[m
[32m+[m[32m    RST(state);[m
[32m+[m[32m    break;[m
     [m
     // --------------- INCREMENTS/DECREMENTS --------------------[m
 [m
[36m@@ -785,7 +718,6 @@[m [mvoid Emulate8080Op(State8080_T state) {[m
   case 0x34:               // INR M, increment memory register[m
   case 0x3C:               // INR A, increment accumulator[m
     INR_R (state);[m
[31m-    state->pc += 1;[m
     break;[m
 [m
     // ---------------------- DCR -------------------------------[m
[36m@@ -798,83 +730,76 @@[m [mvoid Emulate8080Op(State8080_T state) {[m
   case 0x35:               // DCR M, decrement memory register[m
   case 0x3D:               // DCR A, decrement accumulator[m
     DCR_R (state);[m
[31m-    state->pc += 1;[m
     break;[m
 [m
     // ---------------------- INX -------------------------------[m
   case 0x03:               // INX B, increment BC[m
     {[m
[31m-      uint16_t BC = (state->registers[0]<<8) | (state->registers[1] & 0xff);[m
[32m+[m[32m      uint16_t BC = (state->registers[0]<<8) | (state->registers[1]);[m
       [m
[31m-      uint16_t sum = BC + 1;[m
[32m+[m[32m      uint32_t sum = BC++;[m
       // populate bc[m
       state->registers[0] = (uint8_t) (sum>>8);[m
[31m-      state->registers[1] = (uint8_t) (sum & 0xff);[m
[32m+[m[32m      state->registers[1] = (uint8_t) (sum & 0x0f);[m
       // no flags affected[m
     }[m
[31m-    state->pc += 1;[m
     break;[m
   case 0x13:               // INX D, increment DE[m
     {[m
[31m-      uint16_t DE = (state->registers[2]<<8) | (state->registers[3] & 0xff);[m
[32m+[m[32m      uint16_t DE = (state->registers[2]<<8) | (state->registers[3]);[m
       [m
[31m-      uint16_t sum = DE + 1;[m
[32m+[m[32m      uint32_t sum = DE++;[m
       // populate DE[m
       state->registers[2] = (uint8_t) (sum>>8);[m
[31m-      state->registers[3] = (uint8_t) (sum & 0xff);[m
[32m+[m[32m      state->registers[3] = (uint8_t) (sum & 0x0f);[m
       // no flags affected[m
     }[m
[31m-    state->pc += 1;[m
     break;[m
   case 0x23:               // INX H, increment HL[m
     {[m
[31m-      uint16_t HL = (state->registers[4]<<8) | (state->registers[5] & 0xff);[m
[32m+[m[32m      uint16_t HL = (state->registers[4]<<8) | (state->registers[5]);[m
       [m
[31m-      uint16_t sum = HL + 1;[m
[32m+[m[32m      uint32_t sum = HL++;[m
       // populate HL[m
       state->registers[4] = (uint8_t) (sum>>8);[m
[31m-      state->registers[5] = (uint8_t) (sum & 0xff);[m
[32m+[m[32m      state->registers[5] = (uint8_t) (sum & 0x0f);[m
       // no flags affected[m
     }[m
[31m-    state->pc += 1;[m
     break;[m
       [m
     // ----------------------- DCX -----------------------------[m
   case 0x0B:               // DCX B, decrement BC[m
     {[m
[31m-      uint16_t BC = (state->registers[0]<<8) | (state->registers[1] & 0xff);[m
[32m+[m[32m      uint16_t BC = (state->registers[0]<<8) | (state->registers[1]);[m
       [m
[31m-      uint16_t sum = BC - 1;[m
[32m+[m[32m      uint32_t sum = BC--;[m
       // populate bc[m
       state->registers[0] = (uint8_t) (sum>>8);[m
[31m-      state->registers[1] = (uint8_t) (sum & 0xff);[m
[32m+[m[32m      state->registers[1] = (uint8_t) (sum & 0x0f);[m
       // no flags affected[m
     }[m
[31m-    state->pc += 1;[m
     break;[m
   case 0x1B:               // DCX D, decrement DE[m
     {[m
[31m-      uint16_t DE = (state->registers[2]<<8) | (state->registers[3] & 0xff);[m
[32m+[m[32m      uint16_t DE = (state->registers[2]<<8) | (state->registers[3]);[m
       [m
[31m-      uint16_t sum = DE - 1;[m
[32m+[m[32m      uint32_t sum = DE--;[m
       // populate de[m
       state->registers[2] = (uint8_t) (sum>>8);[m
[31m-      state->registers[3] = (uint8_t) (sum & 0xff);[m
[32m+[m[32m      state->registers[3] = (uint8_t) (sum & 0x0f);[m
       // no flags affected[m
     }[m
[31m-    state->pc += 1;[m
     break;[m
   case 0x2B:               // DCX H, decrement HL[m
     {[m
[31m-      uint16_t HL = (state->registers[4]<<8) | (state->registers[5] & 0xff);[m
[32m+[m[32m      uint16_t HL = (state->registers[4]<<8) | (state->registers[5]);[m
       [m
[31m-      uint16_t sum = HL - 1;[m
[32m+[m[32m      uint32_t sum = HL--;[m
       // populate hl[m
       state->registers[4] = (uint8_t) (sum>>8);[m
[31m-      state->registers[5] = (uint8_t) (sum & 0xff);[m
[32m+[m[32m      state->registers[5] = (uint8_t) (sum & 0x0f);[m
       // no flags affected[m
     }[m
[31m-    state->pc += 1;[m
     break;[m
 [m
     // ------------------------ADDS------------------------------[m
[36m@@ -889,7 +814,6 @@[m [mvoid Emulate8080Op(State8080_T state) {[m
   case 0x86:               // ADD M, add from memory ("memory register")[m
   case 0x87:               // ADD accumulator[m
     ADD_R(state);[m
[31m-    state->pc += 1;[m
     break;[m
     [m
     // ----------------------- ADC -------------------------------[m
[36m@@ -902,7 +826,6 @@[m [mvoid Emulate8080Op(State8080_T state) {[m
   case 0x8E:               // ADC M, add memory to A with carry[m
   case 0x8F:               // ADC A, add accumulator to A with carry[m
     ADC_R(state);[m
[31m-    state->pc += 1;[m
     break;[m
 [m
     // ---------------------- ADI --------------------------------[m
[36m@@ -914,10 +837,9 @@[m [mvoid Emulate8080Op(State8080_T state) {[m
       state->registers[7] = sum & 0xff;[m
       state->pc += 1; // for the immediate byte[m
     }[m
[31m-    state->pc += 1;[m
     break;[m
 [m
[31m-  case 0xC7:               // ACI, add immediate to A with carry[m
[32m+[m[32m  case 0xCE:               // ACI, add immediate to A with carry[m
     {[m
       uint16_t sum = (uint16_t) state->registers[7] + (uint16_t) opcode[1] + (uint16_t) state->cc->c;[m
       [m
[36m@@ -925,69 +847,62 @@[m [mvoid Emulate8080Op(State8080_T state) {[m
       state->registers[7] = sum & 0xff;[m
       state->pc += 1;[m
     }[m
[31m-    state->pc += 1;[m
     break;[m
 [m
     // ------------------------ DAD ---------------------------[m
   case 0x09:              // DAD B, BC + HL --> HL, updates only carry flag[m
     {[m
[31m-      uint16_t BC = (state->registers[0]<<8) | (state->registers[1] & 0xff);[m
[31m-      uint16_t HL = (state->registers[4]<<8) | (state->registers[5] & 0xff);[m
[32m+[m[32m      uint16_t BC = (state->registers[0]<<8) | (state->registers[1]);[m
[32m+[m[32m      uint16_t HL = (state->registers[4]<<8) | (state->registers[5]);[m
       [m
       uint32_t sum = BC + HL;[m
       // populate HL[m
[31m-      state->registers[4] = (uint8_t) ((sum & 0x0000ffff)>>8);[m
[31m-      state->registers[5] = (uint8_t) (sum & 0x000000ff);[m
[32m+[m[32m      state->registers[4] = (uint8_t) (sum>>8);[m
[32m+[m[32m      state->registers[5] = (uint8_t) (sum & 0x0f);[m
       [m
       // update carry flag[m
       state->cc->c = (sum > 0xffff);[m
     }[m
[31m-    state->pc += 1;[m
     break;[m
   case 0x19:              // DAD D, DE + HL --> HL, updates only carry flag[m
     {[m
[31m-      uint16_t DE = (state->registers[2]<<8) | (state->registers[3] & 0xff);[m
[31m-      uint16_t HL = (state->registers[4]<<8) | (state->registers[5] & 0xff);[m
[32m+[m[32m      uint16_t DE = (state->registers[2]<<8) | (state->registers[3]);[m
[32m+[m[32m      uint16_t HL = (state->registers[4]<<8) | (state->registers[5]);[m
       [m
       uint32_t sum = DE + HL;[m
[31m-      [m
       // populate HL[m
[31m-      state->registers[4] = (uint8_t) ((sum & 0x0000ffff)>>8);[m
[31m-      state->registers[5] = (uint8_t) (sum & 0x000000ff);[m
[32m+[m[32m      state->registers[4] = (uint8_t) (sum>>8);[m
[32m+[m[32m      state->registers[5] = (uint8_t) (sum & 0x0f);[m
       [m
       // update carry flag[m
       state->cc->c = (sum > 0xffff);[m
     }[m
[31m-    state->pc += 1;[m
     break;[m
   case 0x29:              // DAD H, HL + HL --> HL, updates only carry flag[m
     {[m
[31m-      uint16_t HL = (state->registers[4]<<8) | (state->registers[5] & 0xff);[m
[32m+[m[32m      uint16_t HL = (state->registers[4]<<8) | (state->registers[5]);[m
       [m
       uint32_t sum = HL + HL;[m
[31m-[m
       // populate HL[m
[31m-      state->registers[4] = (uint8_t) ((sum & 0x0000ffff)>>8);[m
[31m-      state->registers[5] = (uint8_t) (sum & 0x000000ff);[m
[32m+[m[32m      state->registers[4] = (uint8_t) (sum>>8);[m
[32m+[m[32m      state->registers[5] = (uint8_t) (sum & 0x0f);[m
       [m
       // update carry flag[m
       state->cc->c = (sum > 0xffff);[m
     }[m
[31m-    state->pc += 1;[m
     break;[m
   case 0x39:              // DAD SP, SP + HL --> HL, updates only carry flag[m
     {[m
[31m-      uint16_t HL = (state->registers[4]<<8) | (state->registers[5] & 0xff);[m
[32m+[m[32m      uint16_t HL = (state->registers[4]<<8) | (state->registers[5]);[m
       [m
       uint32_t sum = state->sp + HL;[m
       // populate HL[m
[31m-      state->registers[4] = (uint8_t) ((sum & 0x0000ffff)>>8);[m
[31m-      state->registers[5] = (uint8_t) (sum & 0x000000ff);[m
[32m+[m[32m      state->registers[4] = (uint8_t) (sum>>8);[m
[32m+[m[32m      state->registers[5] = (uint8_t) (sum & 0x0f);[m
       [m
       // update carry flag[m
       state->cc->c = (sum > 0xffff);[m
     }[m
[31m-    state->pc += 1;[m
     break;[m
 [m
     // ------------------- SUBTRACTS --------------------------[m
[36m@@ -1004,7 +919,6 @@[m [mvoid Emulate8080Op(State8080_T state) {[m
   case 0xA6:              // ANA Memory, AND register with A, goes into A[m
   case 0xA7:              // ANA A, AND register with A, goes into A[m
     ANA (state);[m
[31m-    state->pc += 1;[m
     break;[m
 [m
     // ----------------------- XRA ----------------------------[m
[36m@@ -1017,7 +931,6 @@[m [mvoid Emulate8080Op(State8080_T state) {[m
   case 0xAE:              // XRA Memory, XOR register with A, goes into A[m
   case 0xAF:              // XRA A, XOR register with A, goes into A[m
     XRA (state);[m
[31m-    state->pc += 1;[m
     break;[m
 [m
     // ----------------------- ANI ----------------------------[m
[36m@@ -1032,20 +945,18 @@[m [mvoid Emulate8080Op(State8080_T state) {[m
       [m
       state->pc += 1;[m
     }[m
[31m-    state->pc += 1;[m
     break;[m
 [m
     // ------------------------ CPI ---------------------------[m
   case 0xFE:              // CPI, compare immediate with A[m
     // condition bits set by (A) - Data[m
     {[m
[31m-      uint16_t result = (state->registers[7] - opcode[1]) & 0xffff;[m
[32m+[m[32m      uint8_t result = state->registers[7] - opcode[1];[m
       [m
       UpdateCCSimple(state, result);[m
       [m
       state->pc += 1;[m
     }[m
[31m-    state->pc += 1;[m
     break;[m
     [m
     // ------------------- ROTATE -----------------------------[m
[36m@@ -1064,7 +975,6 @@[m [mvoid Emulate8080Op(State8080_T state) {[m
       state->registers[7] &= 0x7f;         // clear last bit[m
       state->registers[7] |= (a0 << 7);    // update last bit[m
     }[m
[31m-    state->pc += 1;[m
     break;[m
 [m
     // ------------------ INPUT/OUTPUT -------------------------[m
[36m@@ -1078,7 +988,6 @@[m [mvoid Emulate8080Op(State8080_T state) {[m
 [m
       state->registers[7] = (*state->drivers->in[port]) ();[m
       state->pc += 1;[m
[31m-      state->pc += 1;[m
     }[m
     break;[m
   case 0xD3:              // OUT, output[m
[36m@@ -1086,12 +995,7 @@[m [mvoid Emulate8080Op(State8080_T state) {[m
     {[m
       uint8_t port = opcode[1];[m
 [m
[31m-      printf ("port : $%02x", port);[m
[31m-      fflush(stdout);[m
[31m-      void (*p) (uint8_t i) = state->drivers->out[port];[m
[31m-[m
[31m-      (*p) (state->registers[7]);[m
[31m-      state->pc += 1;[m
[32m+[m[32m      (*state->drivers->out[port]) (state->registers[7]);[m
       state->pc += 1;[m
     }[m
     break;[m
[36m@@ -1099,11 +1003,9 @@[m [mvoid Emulate8080Op(State8080_T state) {[m
     // --------------------- CONTROL -----------------------------[m
   case 0xFB:              // EL, enable interrupts[m
     state->IE = 1;[m
[31m-    state->pc += 1;[m
     break;[m
   case 0xF3:              // DI, disable interrupts[m
     state->IE = 0;[m
[31m-    state->pc += 1;[m
     break;[m
   case 0x76:              // HLT, halt[m
     printf ("Recieved halt");[m
[36m@@ -1113,17 +1015,26 @@[m [mvoid Emulate8080Op(State8080_T state) {[m
     UnimplementedInstruction(state);[m
   }[m
   [m
[32m+[m[32m  state->pc += 1; // move up one for opcode itself[m
[32m+[m
   //  PRINT CURRENT INSTRUCTION AND STATE[m
   printf("\t CURRENT INSTRUCTION: $%02x\n", *opcode);[m
   [m
   printf("\t CURRENT STATE: \n");[m
   [m
   printf("\tC=%d, P=%d, S=%d, Z = %d\n", state->cc->c, state->cc->p, state->cc->s, state->cc->z);[m
[31m-  printf("\tA $%02x B $%02x C $%02x D $%02x E $%02x H $%02x L $%02x SP %04x PC  %04x\n", state->registers[7], state->registers[0], state->registers[1], state->registers[2], state->registers[3], state->registers[4], state->registers[5], state->sp, state->pc);[m
[32m+[m[32m  printf("\tA $%02x B $%02x C $%02x D $%02x E $%02x H $%02x L $%02x SP %04x\n", state->registers[7], state->registers[0], state->registers[1], state->registers[2], state->registers[3], state->registers[4], state->registers[5], state->sp);[m
 [m
   printf("\n");[m
[32m+[m[32m}[m
 [m
[31m-  fflush(stdout);[m
[32m+[m[32mvoid Emulate8080State(State8080_T state) {[m
[32m+[m[32m  unsigned char *opcode;[m
[32m+[m[41m  [m
[32m+[m[32m  // get opcode from memory at PC, PC steps away from start of memory[m
[32m+[m[32m  opcode = &state->memory[state->pc];[m
[32m+[m
[32m+[m[32m  Emulate8080Op(state, opcode);[m
 }[m
 [m
 ConditionCodes_T ConditionCodes_init () {[m
[36m@@ -1154,6 +1065,7 @@[m [mState8080_T State8080_init () {[m
   state->sp = 0;[m
   state->pc = 0;[m
   state->memory = (uint8_t *) malloc (65535 * sizeof (uint8_t));[m
[32m+[m[32m  state->interrupts = (uint8_t *) malloc (sizeof(uint8_t));[m
 [m
   state->cc = ConditionCodes_init();[m
 [m
[36m@@ -1173,52 +1085,42 @@[m [mvoid empty_OUT (uint8_t ac) {[m
 Drivers_T Drivers_init () {[m
   Drivers_T drivers = (struct Drivers *) malloc (sizeof (struct Drivers));[m
 [m
[31m-  drivers->in[0] = &empty_IN;[m
[31m-  drivers->in[1] = &empty_IN;[m
[31m-  drivers->in[2] = &empty_IN;[m
[31m-  drivers->in[3] = &empty_IN;[m
[31m-  drivers->in[4] = &empty_IN;[m
[31m-  drivers->in[5] = &empty_IN;[m
[31m-  drivers->in[6] = &empty_IN;[m
[31m-  drivers->in[7] = &empty_IN;[m
[31m-[m
[31m-  drivers->out[0] = &empty_OUT;[m
[31m-  drivers->out[1] = &empty_OUT;[m
[31m-  drivers->out[2] = &empty_OUT;[m
[31m-  drivers->out[3] = &empty_OUT;[m
[31m-  drivers->out[4] = &empty_OUT;[m
[31m-  drivers->out[5] = &empty_OUT;[m
[31m-  drivers->out[6] = &empty_OUT;[m
[31m-  drivers->out[7] = &empty_OUT;[m
[32m+[m[32m  drivers->in[0] = empty_IN;[m
[32m+[m[32m  drivers->in[1] = empty_IN;[m
[32m+[m[32m  drivers->in[2] = empty_IN;[m
[32m+[m[32m  drivers->in[3] = empty_IN;[m
[32m+[m[32m  drivers->in[4] = empty_IN;[m
[32m+[m[32m  drivers->in[5] = empty_IN;[m
[32m+[m[32m  drivers->in[6] = empty_IN;[m
[32m+[m[32m  drivers->in[7] = empty_IN;[m
[32m+[m
[32m+[m[32m  drivers->out[0] = empty_OUT;[m
[32m+[m[32m  drivers->out[1] = empty_OUT;[m
[32m+[m[32m  drivers->out[2] = empty_OUT;[m
[32m+[m[32m  drivers->out[3] = empty_OUT;[m
[32m+[m[32m  drivers->out[4] = empty_OUT;[m
[32m+[m[32m  drivers->out[5] = empty_OUT;[m
[32m+[m[32m  drivers->out[6] = empty_OUT;[m
[32m+[m[32m  drivers->out[7] = empty_OUT;[m
 [m
   return drivers;[m
 }[m
 [m
 void State8080_load_mem(State8080_T state, int start, unsigned char *buffer) {[m
[31m-  for (int i = start; i < 8192; i++) {[m
[32m+[m[32m  for (int i = start; i < 2048; i++) {[m
     state->memory[i] = (uint8_t) *buffer;[m
[31m-    if (i == 6355) {[m
[31m-      printf ("$%02x", state->memory[i]);[m
[31m-    }[m
     buffer++;[m
   }[m
 }[m
 [m
[31m-void State8080_config_drivers_in_port(State8080_T state, uint8_t (*in) (), uint8_t port) {[m
[31m-  state->drivers->in[port] = in;[m
[31m-}[m
[31m-[m
[31m-void State8080_config_drivers_out_port(State8080_T state, void (*out) (uint8_t), uint8_t port) {[m
[31m-  state->drivers->out[port] = out;[m
[32m+[m[32muint8_t State8080_ie(State8080_T state) {[m
[32m+[m[32m  return state->IE;[m
 }[m
 [m
[31m-void State8080_config_drivers_default(State8080_T state, Drivers_T drivers) {[m
[31m-  state->drivers = drivers;[m
[31m-}[m
[31m-void config_drivers_in_port(Drivers_T drivers, uint8_t (*in) (), uint8_t port) {[m
[31m-  drivers->in[port] = in;[m
[32m+[m[32mvoid State8080_pushInterrupt(State8080_T state, uint8_t int_num) {[m
[32m+[m[32m  state->interrupts[0] = int_num;[m
 }[m
 [m
[31m-void config_drivers_out_port(Drivers_T drivers, void (*out) (uint8_t), uint8_t port) {[m
[31m-  drivers->out[port] = out;[m
[32m+[m[32muint8_t State8080_popInterrupt(State8080_T state) {[m
[32m+[m[32m  return state->interrupts[0];[m
 }[m
[1mdiff --git a/CPU.c~ b/CPU.c~[m
[1mnew file mode 100644[m
[1mindex 0000000..8aefb44[m
[1m--- /dev/null[m
[1m+++ b/CPU.c~[m
[36m@@ -0,0 +1,8 @@[m
[32m+[m[32m/*[m
[32m+[m[32m  File name : CPU.c[m
[32m+[m[41m  [m
[32m+[m[32m  Description : contains the bulk of the information for the 8080 cpu[m
[32m+[m[32m  Author : Antony Toron[m
[32m+[m
[32m+[m[32m */[m
[32m+[m
[1mdiff --git a/CPU.h b/CPU.h[m
[1mindex 1cd1655..f35474c 100644[m
[1m--- a/CPU.h[m
[1m+++ b/CPU.h[m
[36m@@ -11,7 +11,9 @@[m [mtypedef struct State8080* State8080_T;[m
 typedef struct ConditionCodes* ConditionCodes_T;[m
 typedef struct Drivers* Drivers_T;[m
 [m
[31m-void Emulate8080Op(State8080_T state);[m
[32m+[m[32mvoid Emulate8080Op(State8080_T state, unsigned char *opcode);[m
[32m+[m
[32m+[m[32mvoid Emulate8080State(State8080_T state);[m
 [m
 State8080_T State8080_init();[m
 [m
[36m@@ -21,12 +23,8 @@[m [mDrivers_T Drivers_init();[m
 [m
 void State8080_load_mem(State8080_T state, int start, unsigned char *buffer);[m
 [m
[31m-void State8080_config_drivers_in_port(State8080_T state, uint8_t (*in) (), uint8_t port);[m
[31m-[m
[31m-void State8080_config_drivers_out_port(State8080_T state, void (*out) (uint8_t), uint8_t port);[m
[31m-[m
[31m-void State8080_config_drivers_default(State8080_T state, Drivers_T drivers);[m
[32m+[m[32muint8_t State8080_ie(State8080_T state);[m
 [m
[31m-void config_drivers_in_port(Drivers_T drivers, uint8_t (*in) (), uint8_t port);[m
[32m+[m[32mvoid State8080_pushInterrupt(State8080_T state, uint8_t int_num);[m
 [m
[31m-void config_drivers_out_port(Drivers_T drivers, void (*out) (uint8_t), uint8_t port);[m
[32m+[m[32muint8_t State8080_popInterrupt(State8080_T state);[m
[1mdiff --git a/CPU.h~ b/CPU.h~[m
[1mnew file mode 100644[m
[1mindex 0000000..9c81ec7[m
[1m--- /dev/null[m
[1m+++ b/CPU.h~[m
[36m@@ -0,0 +1,16 @@[m
[32m+[m[32m/*[m
[32m+[m[32m  File name : CPU.h[m
[32m+[m[41m  [m
[32m+[m[32m  Description : Header file for 8080 processor.[m
[32m+[m[32m  Author : Antony Toron[m
[32m+[m[32m */[m
[32m+[m
[32m+[m[32mtypedef struct State8080* State8080_T;[m
[32m+[m[32mtypedef struct ConditionCodes* ConditionCodes_T;[m
[32m+[m[32mtypedef struct Drivers* Drivers_T;[m
[32m+[m
[32m+[m[32mvoid Emulate8080Op(State8080_T state);[m
[32m+[m
[32m+[m[32mState8080_T State8080_init();[m
[32m+[m
[32m+[m[32mConditionCodes_T ConditionCodes_init();[m
[1mdiff --git a/CPU.o b/CPU.o[m
[1mnew file mode 100644[m
[1mindex 0000000..25dedec[m
Binary files /dev/null and b/CPU.o differ
[1mdiff --git a/Drivers.c b/Drivers.c[m
[1mdeleted file mode 100644[m
[1mindex 49c0d02..0000000[m
[1m--- a/Drivers.c[m
[1m+++ /dev/null[m
[36m@@ -1,61 +0,0 @@[m
[31m-/*[m
[31m-  File name : Drivers.c[m
[31m-  [m
[31m-  Description : Returns functions to perform for ports, contains all[m
[31m-         information about ports[m
[31m-  Author : Antony Toron[m
[31m-*/[m
[31m-[m
[31m-#include "Drivers.h"[m
[31m-#include <stdint.h>[m
[31m-#include <stdio.h>[m
[31m-#include <stdlib.h>[m
[31m-[m
[31m-// -------------------- ARCADE MACHINE ---------------------[m
[31m-[m
[31m-struct ArcadeMachinePorts {[m
[31m-  uint8_t shift_registers[2];[m
[31m-  uint8_t offset;[m
[31m-  [m
[31m-  // fill in other necessary memory[m
[31m-};[m
[31m-[m
[31m-ArcadeMachinePorts_T am_ports_init () {[m
[31m-  ArcadeMachinePorts_T am_ports = (struct ArcadeMachinePorts *) malloc (sizeof (struct ArcadeMachinePorts));[m
[31m-[m
[31m-  am_ports->shift_registers[0] = 0;[m
[31m-  am_ports->shift_registers[1] = 0;[m
[31m-  am_ports->offset = 0;[m
[31m-  [m
[31m-  return am_ports;[m
[31m-}[m
[31m-[m
[31m-// -------------------- SHIFT REGISTER ----------------------[m
[31m-[m
[31m-// 8080 Arcade port 4: (Write), sets data in shift registers[m
[31m-// [8 bits a][8 bits b], shifts a into b, and the new value written to b[m
[31m-void Arcade8080_write4 (uint8_t ac, ArcadeMachinePorts_T am_ports) {[m
[31m-  am_ports->shift_registers[1] = am_ports->shift_registers[0];[m
[31m-  am_ports->shift_registers[1] = ac;[m
[31m-}[m
[31m-[m
[31m-// 8080 Arcade port 2: (Write), sets the shift amount, DOESNT RETURN IT[m
[31m-// bits 0, 1, 2 set offset for 8 bit result[m
[31m-// i.e.[m
[31m-// xxxxxxxx:yyyyyyyy -> offset 2: 8 bit result = xxxxxxyy[m
[31m-// offset 7 : 8 bit result = xyyyyyyy[m
[31m-void Arcade8080_write2 (uint8_t ac, ArcadeMachinePorts_T am_ports) {[m
[31m-  am_ports->offset = 0;[m
[31m-}[m
[31m-[m
[31m-// 8080 Arcade port 3: (Read) // reads data based on shift[m
[31m-// returns result from port 2 writes[m
[31m-uint8_t Arcade8080_read3 (ArcadeMachinePorts_T am_ports) {[m
[31m-  uint8_t result = 0;[m
[31m-[m
[31m-  [m
[31m-[m
[31m-  return result;[m
[31m-}[m
[31m-[m
[31m-[m
[1mdiff --git a/Drivers.h b/Drivers.h[m
[1mdeleted file mode 100644[m
[1mindex 3f30ea8..0000000[m
[1m--- a/Drivers.h[m
[1m+++ /dev/null[m
[36m@@ -1,19 +0,0 @@[m
[31m-/*[m
[31m-  File name : Drivers.h[m
[31m-  [m
[31m-  Description : Drivers for all types of 8080 machines[m
[31m-  Author : Antony Toron[m
[31m-[m
[31m- */[m
[31m-[m
[31m-#include <stdint.h>[m
[31m-[m
[31m-typedef struct ArcadeMachinePorts * ArcadeMachinePorts_T;[m
[31m-[m
[31m-ArcadeMachinePorts_T am_ports_init ();[m
[31m-[m
[31m-void Arcade8080_write4 (uint8_t ac, ArcadeMachinePorts_T am_ports);[m
[31m-[m
[31m-void Arcade8080_write2 (uint8_t ac, ArcadeMachinePorts_T am_ports);[m
[31m-[m
[31m-uint8_t Arcade8080_read3 (ArcadeMachinePorts_T am_ports);[m
[1mdiff --git a/Makefile b/Makefile[m
[1mindex 5a5812b..1a488a8 100644[m
[1m--- a/Makefile[m
[1m+++ b/Makefile[m
[36m@@ -5,18 +5,21 @@[m
 [m
 # Macros[m
 CC = gcc[m
[31m-[m
[31m-#CFLAGS =[m
[31m-CFLAGS = -g[m
[32m+[m[32mCPP = g++[m
[32m+[m[32mCFLAGS =[m
[32m+[m[32m# CFLAGS = -g[m
 # CFLAGS = -D NDEBUG[m
 # CFLAGS = -D NDEBUG -O[m
[32m+[m[32mCPPFLAGS = -std=c++11[m
[32m+[m
[32m+[m[32mLIBS = -lGL -lGLEW -lglfw -lglut -lGLU -lpthread[m
 [m
 # Pattern rule, any .o file with .c file of same name will assume it[m
 # %.o: %.c[m
 #       $ (CC) $ (CFLAGS) -c $<[m
 [m
 # Dependency rules for non-file targets[m
[31m-all: disassemble arcade[m
[32m+[m[32mall: disassemble c_arcade arcade playground[m
 [m
 # clean directory[m
 clobber: clean[m
[36m@@ -26,20 +29,29 @@[m [mclean:[m
 [m
 # Dependency rules for file targets[m
 # include all files ncessary for building in -o shortcut case[m
[31m-arcade: 8080Arcade.o CPU.o Utils.o Drivers.o[m
[31m-	$(CC) $(CFLAGS) $< CPU.o Utils.o Drivers.o -o $@[m
[32m+[m[32mc_arcade: 8080Arcade.o CPU.o Utils.o[m
[32m+[m	[32m$(CC) $(CFLAGS) $< CPU.o Utils.o -o $@[m
 disassemble: disassemble.o disassembler.o[m
 	$(CC) $(CFLAGS) $< disassembler.o -o $@[m
 [m
[32m+[m[32mplayground: playground.o[m
[32m+[m	[32m$(CPP) $(CFLAGS) $< -o $@ $(LIBS)[m
[32m+[m[32marcade: arcade_machine.o CPU.o Utils.o[m
[32m+[m	[32m$(CPP) $(CFLAGS) $< CPU.o Utils.o -o $@ $(LIBS)[m
[32m+[m
 # object file dependencies in recipes for all binary files[m
[31m-8080Arcade.o: 8080Arcade.c CPU.h Utils.h Drivers.h[m
[32m+[m[32m8080Arcade.o: 8080Arcade.c CPU.h Utils.h[m
 	$(CC) $(CFLAGS) -c $<[m
 CPU.o: CPU.c CPU.h[m
 	$(CC) $(CFLAGS) -c $<[m
 Utils.o: Utils.c Utils.h[m
 	$(CC) $(CFLAGS) -c $<[m
[31m-Drivers.o: Drivers.c Drivers.h[m
[31m-	$(CC) $(CFLAGS) -c $<[m
[32m+[m
[32m+[m[32mplayground.o: playground.cpp[m
[32m+[m	[32m$(CPP) $(CFLAGS) -c $< $(LIBS)[m
[32m+[m[32marcade_machine.o: arcade_machine.cpp[m
[32m+[m	[32m$(CPP) $(CPPFLAGS) -c $< $(LIBS)[m
[32m+[m
 [m
 disassemble.o: disassemble.c disassembler.h[m
 	$(CC) $(CFLAGS) -c $<[m
[1mdiff --git a/Makefile~ b/Makefile~[m
[1mnew file mode 100644[m
[1mindex 0000000..6c45e53[m
[1m--- /dev/null[m
[1m+++ b/Makefile~[m
[36m@@ -0,0 +1,36 @@[m
[32m+[m[32m#--------------[m
[32m+[m[32m# Makefile for 8080 Processor[m
[32m+[m[32m# Author : Antony Toron[m
[32m+[m[32m#--------------[m
[32m+[m
[32m+[m[32m# Macros[m
[32m+[m[32mCC = gcc[m
[32m+[m
[32m+[m[32mCFLAGS =[m
[32m+[m[32m# CFLAGS = -g[m
[32m+[m[32m# CFLAGS = -D NDEBUG[m
[32m+[m[32m# CFLAGS = -D NDEBUG -O[m
[32m+[m
[32m+[m[32m# Pattern rule, any .o file with .c file of same name will assume it[m
[32m+[m[32m# %.o: %.c[m
[32m+[m[32m#       $ (CC) $ (CFLAGS) -c $<[m
[32m+[m
[32m+[m[32m# Dependency rules for non-file targets[m
[32m+[m[32mall: disassemble[m
[32m+[m
[32m+[m[32m# clean directory[m
[32m+[m[32mclobber: clean[m
[32m+[m	[32mrm -f *~ \#*\#[m
[32m+[m[32mclean:[m
[32m+[m	[32mrm -f disassemble *.o[m
[32m+[m
[32m+[m[32m# Dependency rules for file targets[m
[32m+[m[32m# include all files ncessary for building in -o shortcut case[m
[32m+[m[32mdisassemble: disassemble.o disassembler.o[m
[32m+[m	[32m$(CC) $(CFLAGS) $< disassembler.o -o $@[m
[32m+[m
[32m+[m[32m# object file dependencies in recipes for all binary files[m
[32m+[m[32mdisassemble.o: disassemble.c disassemble.h[m
[32m+[m	[32m$(CC) $(CFLAGS) -c $<[m
[32m+[m[32mdisassembler.o: disassembler.c disassemble.h[m
[32m+[m	[32m$(CC) $(CFLAGS) -c $<[m
[1mdiff --git a/Utils.c b/Utils.c[m
[1mindex 37a6cf0..3044565 100644[m
[1m--- a/Utils.c[m
[1m+++ b/Utils.c[m
[36m@@ -10,7 +10,7 @@[m
 #include <stddef.h>[m
 #include <stdlib.h>[m
 [m
[31m-unsigned char * readFileIntoBuffer(char *filename) {[m
[32m+[m[32munsigned char * readFileIntoBuffer(const char *filename) {[m
   FILE *f = fopen (filename, "rb");[m
   //printf ("Opening the file\n");[m
 [m
[36m@@ -27,8 +27,6 @@[m [munsigned char * readFileIntoBuffer(char *filename) {[m
   unsigned char *buffer = malloc(fsize * sizeof(char));[m
 [m
   size_t bytesRead = fread(buffer, 1, fsize, f);[m
[31m-[m
[31m-  printf ("Bytes read: %i\n", (int) bytesRead);[m
   fclose(f);[m
 [m
   return buffer;[m
[1mdiff --git a/Utils.c~ b/Utils.c~[m
[1mnew file mode 100644[m
[1mindex 0000000..026bce3[m
[1m--- /dev/null[m
[1m+++ b/Utils.c~[m
[36m@@ -0,0 +1,28 @@[m
[32m+[m[32m/*[m
[32m+[m[32m  File name : Utils.c[m
[32m+[m
[32m+[m[32m  Description : Misc helper utils for 8080 processor and ancilliary tools[m
[32m+[m[32m  Author : Antony Toron[m
[32m+[m[32m */[m
[32m+[m
[32m+[m[32munsigned char * readFileIntoBuffer(char *filename) {[m
[32m+[m[32m  FILE *f = fopen (filename, "rb");[m
[32m+[m[32m  //printf ("Opening the file\n");[m
[32m+[m
[32m+[m[32m  if (f == NULL) {[m
[32m+[m[32m    fprintf (stderr, "Couldn't open %s\n", filename);[m
[32m+[m[32m    exit(1);[m
[32m+[m[32m  }[m
[32m+[m
[32m+[m[32m  // get file size and read into buffer[m
[32m+[m[32m  fseek(f, 0L, SEEK_END);[m
[32m+[m[32m  int fsize = ftell(f);[m
[32m+[m[32m  fseek(f, 0L, SEEK_SET);[m
[32m+[m
[32m+[m[32m  unsigned char *buffer = malloc(fsize * sizeof(char));[m
[32m+[m
[32m+[m[32m  size_t bytesRead = fread(buffer, 1, fsize, f);[m
[32m+[m[32m  fclose(f);[m
[32m+[m
[32m+[m[32m  return buffer;[m
[32m+[m[32m}[m
[1mdiff --git a/Utils.h b/Utils.h[m
[1mindex 8c03a70..1fa7e8a 100644[m
[1m--- a/Utils.h[m
[1m+++ b/Utils.h[m
[36m@@ -6,4 +6,4 @@[m
  */[m
 [m
 [m
[31m-unsigned char * readFileIntoBuffer(char *filename);[m
[32m+[m[32munsigned char * readFileIntoBuffer(const char *filename);[m
[1mdiff --git a/Utils.h~ b/Utils.h~[m
[1mnew file mode 100644[m
[1mindex 0000000..9dc97d9[m
[1m--- /dev/null[m
[1m+++ b/Utils.h~[m
[36m@@ -0,0 +1,9 @@[m
[32m+[m[32m/*[m
[32m+[m[32m  File name : Utils.h[m
[32m+[m
[32m+[m[32m  Description : Header for Misc helper utils[m
[32m+[m[32m  Author : Antony Toron[m
[32m+[m[32m */[m
[32m+[m
[32m+[m
[32m+[m[32munsigned char *[m[41m [m
[1mdiff --git a/Utils.o b/Utils.o[m
[1mnew file mode 100644[m
[1mindex 0000000..cf75228[m
Binary files /dev/null and b/Utils.o differ
[1mdiff --git a/arcade b/arcade[m
[1mindex 5d86883..8420a89 100644[m
Binary files a/arcade and b/arcade differ
[1mdiff --git a/disassemble b/disassemble[m
[1mnew file mode 100644[m
[1mindex 0000000..d98fe5c[m
Binary files /dev/null and b/disassemble differ
[1mdiff --git a/disassemble.c~ b/disassemble.c~[m
[1mnew file mode 100644[m
[1mindex 0000000..f4688c7[m
[1m--- /dev/null[m
[1m+++ b/disassemble.c~[m
[36m@@ -0,0 +1,19 @@[m
[32m+[m[32m/*[m
[32m+[m[32m  Run this script on an 8080 code snippet to see the assembly language[m
[32m+[m[32m  translation.[m
[32m+[m
[32m+[m[32m  Author : Antony Toron[m
[32m+[m[32m */[m
[32m+[m
[32m+[m[32m#include "disassembler.h"[m
[32m+[m[32m#include <stdio.h>[m
[32m+[m
[32m+[m[32mint main (int argc, char **argv) {[m
[32m+[m[32m  char *fileName = argv[1];[m
[32m+[m[32m  printf ("%s\n", fileName);[m
[32m+[m[41m  [m
[32m+[m[32m  decode8080CodeFile(fileName);[m
[32m+[m
[32m+[m[32m  printf ("Finished execution");[m
[32m+[m[32m  return 0;[m
[32m+[m[32m}[m
[1mdiff --git a/disassemble.o b/disassemble.o[m
[1mnew file mode 100644[m
[1mindex 0000000..de082da[m
Binary files /dev/null and b/disassemble.o differ
[1mdiff --git a/disassembler.c~ b/disassembler.c~[m
[1mnew file mode 100644[m
[1mindex 0000000..35b2fe7[m
[1m--- /dev/null[m
[1m+++ b/disassembler.c~[m
[36m@@ -0,0 +1,341 @@[m
[32m+[m[32m/*[m
[32m+[m[32m  Disassembler for 8080 processor. Reads in hex stream and prints out[m
[32m+[m[32m  corresponding assembly instruction.[m
[32m+[m
[32m+[m[32m  Author : Antony Toron[m
[32m+[m[32m*/[m
[32m+[m
[32m+[m[32m#include <string.h>[m
[32m+[m[32m#include <stdio.h>[m
[32m+[m[32m#include <stdlib.h>[m
[32m+[m[32m#include "disassembler.h"[m
[32m+[m
[32m+[m[32m// CODE SOURCE : https://github.com/kpmiller/emulator101/blob/master/8080emu-first50.c[m
[32m+[m[32mint Disassemble8080Op(unsigned char *codebuffer, int pc)[m
[32m+[m[32m{[m
[32m+[m[32m  unsigned char *code = &codebuffer[pc];[m
[32m+[m[32m  int opbytes = 1;[m
[32m+[m[32m  printf("%04x ", pc);[m
[32m+[m[32m  switch (*code)[m
[32m+[m[32m    {[m
[32m+[m[32m    case 0x00: printf("NOP"); break;[m
[32m+[m[32m    case 0x01: printf("LXI    B,#$%02x%02x", code[2], code[1]); opbytes=3; break;[m
[32m+[m[32m    case 0x02: printf("STAX   B"); break;[m
[32m+[m[32m    case 0x03: printf("INX    B"); break;[m
[32m+[m[32m    case 0x04: printf("INR    B"); break;[m
[32m+[m[32m    case 0x05: printf("DCR    B"); break;[m
[32m+[m[32m    case 0x06: printf("MVI    B,#$%02x", code[1]); opbytes=2; break;[m
[32m+[m[32m    case 0x07: printf("RLC"); break;[m
[32m+[m[32m    case 0x08: printf("NOP"); break;[m
[32m+[m[32m    case 0x09: printf("DAD    B"); break;[m
[32m+[m[32m    case 0x0a: printf("LDAX   B"); break;[m
[32m+[m[32m    case 0x0b: printf("DCX    B"); break;[m
[32m+[m[32m    case 0x0c: printf("INR    C"); break;[m
[32m+[m[32m    case 0x0d: printf("DCR    C"); break;[m
[32m+[m[32m    case 0x0e: printf("MVI    C,#$%02x", code[1]); opbytes = 2;	break;[m
[32m+[m[32m    case 0x0f: printf("RRC"); break;[m
[32m+[m[41m      [m
[32m+[m[32m    case 0x10: printf("NOP"); break;[m
[32m+[m[32m    case 0x11: printf("LXI    D,#$%02x%02x", code[2], code[1]); opbytes=3; break;[m
[32m+[m[32m    case 0x12: printf("STAX   D"); break;[m
[32m+[m[32m    case 0x13: printf("INX    D"); break;[m
[32m+[m[32m    case 0x14: printf("INR    D"); break;[m
[32m+[m[32m    case 0x15: printf("DCR    D"); break;[m
[32m+[m[32m    case 0x16: printf("MVI    D,#$%02x", code[1]); opbytes=2; break;[m
[32m+[m[32m    case 0x17: printf("RAL"); break;[m
[32m+[m[32m    case 0x18: printf("NOP"); break;[m
[32m+[m[32m    case 0x19: printf("DAD    D"); break;[m
[32m+[m[32m    case 0x1a: printf("LDAX   D"); break;[m
[32m+[m[32m    case 0x1b: printf("DCX    D"); break;[m
[32m+[m[32m    case 0x1c: printf("INR    E"); break;[m
[32m+[m[32m    case 0x1d: printf("DCR    E"); break;[m
[32m+[m[32m    case 0x1e: printf("MVI    E,#$%02x", code[1]); opbytes = 2; break;[m
[32m+[m[32m    case 0x1f: printf("RAR"); break;[m
[32m+[m[41m      [m
[32m+[m[32m    case 0x20: printf("NOP"); break;[m
[32m+[m[32m    case 0x21: printf("LXI    H,#$%02x%02x", code[2], code[1]); opbytes=3; break;[m
[32m+[m[32m    case 0x22: printf("SHLD   $%02x%02x", code[2], code[1]); opbytes=3; break;[m
[32m+[m[32m    case 0x23: printf("INX    H"); break;[m
[32m+[m[32m    case 0x24: printf("INR    H"); break;[m
[32m+[m[32m    case 0x25: printf("DCR    H"); break;[m
[32m+[m[32m    case 0x26: printf("MVI    H,#$%02x", code[1]); opbytes=2; break;[m
[32m+[m[32m    case 0x27: printf("DAA"); break;[m
[32m+[m[32m    case 0x28: printf("NOP"); break;[m
[32m+[m[32m    case 0x29: printf("DAD    H"); break;[m
[32m+[m[32m    case 0x2a: printf("LHLD   $%02x%02x", code[2], code[1]); opbytes=3; break;[m
[32m+[m[32m    case 0x2b: printf("DCX    H"); break;[m
[32m+[m[32m    case 0x2c: printf("INR    L"); break;[m
[32m+[m[32m    case 0x2d: printf("DCR    L"); break;[m
[32m+[m[32m    case 0x2e: printf("MVI    L,#$%02x", code[1]); opbytes = 2; break;[m
[32m+[m[32m    case 0x2f: printf("CMA"); break;[m
[32m+[m[41m      [m
[32m+[m[32m    case 0x30: printf("NOP"); break;[m
[32m+[m[32m    case 0x31: printf("LXI    SP,#$%02x%02x", code[2], code[1]); opbytes=3; break;[m
[32m+[m[32m    case 0x32: printf("STA    $%02x%02x", code[2], code[1]); opbytes=3; break;[m
[32m+[m[32m    case 0x33: printf("INX    SP"); break;[m
[32m+[m[32m    case 0x34: printf("INR    M"); break;[m
[32m+[m[32m    case 0x35: printf("DCR    M"); break;[m
[32m+[m[32m    case 0x36: printf("MVI    M,#$%02x", code[1]); opbytes=2; break;[m
[32m+[m[32m    case 0x37: printf("STC"); break;[m
[32m+[m[32m    case 0x38: printf("NOP"); break;[m
[32m+[m[32m    case 0x39: printf("DAD    SP"); break;[m
[32m+[m[32m    case 0x3a: printf("LDA    $%02x%02x", code[2], code[1]); opbytes=3; break;[m
[32m+[m[32m    case 0x3b: printf("DCX    SP"); break;[m
[32m+[m[32m    case 0x3c: printf("INR    A"); break;[m
[32m+[m[32m    case 0x3d: printf("DCR    A"); break;[m
[32m+[m[32m    case 0x3e: printf("MVI    A,#$%02x", code[1]); opbytes = 2; break;[m
[32m+[m[32m    case 0x3f: printf("CMC"); break;[m
[32m+[m[41m      [m
[32m+[m[32m    case 0x40: printf("MOV    B,B"); break;[m
[32m+[m[32m    case 0x41: printf("MOV    B,C"); break;[m
[32m+[m[32m    case 0x42: printf("MOV    B,D"); break;[m
[32m+[m[32m    case 0x43: printf("MOV    B,E"); break;[m
[32m+[m[32m    case 0x44: printf("MOV    B,H"); break;[m
[32m+[m[32m    case 0x45: printf("MOV    B,L"); break;[m
[32m+[m[32m    case 0x46: printf("MOV    B,M"); break;[m
[32m+[m[32m    case 0x47: printf("MOV    B,A"); break;[m
[32m+[m[32m    case 0x48: printf("MOV    C,B"); break;[m
[32m+[m[32m    case 0x49: printf("MOV    C,C"); break;[m
[32m+[m[32m    case 0x4a: printf("MOV    C,D"); break;[m
[32m+[m[32m    case 0x4b: printf("MOV    C,E"); break;[m
[32m+[m[32m    case 0x4c: printf("MOV    C,H"); break;[m
[32m+[m[32m    case 0x4d: printf("MOV    C,L"); break;[m
[32m+[m[32m    case 0x4e: printf("MOV    C,M"); break;[m
[32m+[m[32m    case 0x4f: printf("MOV    C,A"); break;[m
[32m+[m[41m      [m
[32m+[m[32m    case 0x50: printf("MOV    D,B"); break;[m
[32m+[m[32m    case 0x51: printf("MOV    D,C"); break;[m
[32m+[m[32m    case 0x52: printf("MOV    D,D"); break;[m
[32m+[m[32m    case 0x53: printf("MOV    D.E"); break;[m
[32m+[m[32m    case 0x54: printf("MOV    D,H"); break;[m
[32m+[m[32m    case 0x55: printf("MOV    D,L"); break;[m
[32m+[m[32m    case 0x56: printf("MOV    D,M"); break;[m
[32m+[m[32m    case 0x57: printf("MOV    D,A"); break;[m
[32m+[m[32m    case 0x58: printf("MOV    E,B"); break;[m
[32m+[m[32m    case 0x59: printf("MOV    E,C"); break;[m
[32m+[m[32m    case 0x5a: printf("MOV    E,D"); break;[m
[32m+[m[32m    case 0x5b: printf("MOV    E,E"); break;[m
[32m+[m[32m    case 0x5c: printf("MOV    E,H"); break;[m
[32m+[m[32m    case 0x5d: printf("MOV    E,L"); break;[m
[32m+[m[32m    case 0x5e: printf("MOV    E,M"); break;[m
[32m+[m[32m    case 0x5f: printf("MOV    E,A"); break;[m
[32m+[m[41m      [m
[32m+[m[32m    case 0x60: printf("MOV    H,B"); break;[m
[32m+[m[32m    case 0x61: printf("MOV    H,C"); break;[m
[32m+[m[32m    case 0x62: printf("MOV    H,D"); break;[m
[32m+[m[32m    case 0x63: printf("MOV    H.E"); break;[m
[32m+[m[32m    case 0x64: printf("MOV    H,H"); break;[m
[32m+[m[32m    case 0x65: printf("MOV    H,L"); break;[m
[32m+[m[32m    case 0x66: printf("MOV    H,M"); break;[m
[32m+[m[32m    case 0x67: printf("MOV    H,A"); break;[m
[32m+[m[32m    case 0x68: printf("MOV    L,B"); break;[m
[32m+[m[32m    case 0x69: printf("MOV    L,C"); break;[m
[32m+[m[32m    case 0x6a: printf("MOV    L,D"); break;[m
[32m+[m[32m    case 0x6b: printf("MOV    L,E"); break;[m
[32m+[m[32m    case 0x6c: printf("MOV    L,H"); break;[m
[32m+[m[32m    case 0x6d: printf("MOV    L,L"); break;[m
[32m+[m[32m    case 0x6e: printf("MOV    L,M"); break;[m
[32m+[m[32m    case 0x6f: printf("MOV    L,A"); break;[m
[32m+[m[41m      [m
[32m+[m[32m    case 0x70: printf("MOV    M,B"); break;[m
[32m+[m[32m    case 0x71: printf("MOV    M,C"); break;[m
[32m+[m[32m    case 0x72: printf("MOV    M,D"); break;[m
[32m+[m[32m    case 0x73: printf("MOV    M.E"); break;[m
[32m+[m[32m    case 0x74: printf("MOV    M,H"); break;[m
[32m+[m[32m    case 0x75: printf("MOV    M,L"); break;[m
[32m+[m[32m    case 0x76: printf("HLT");        break;[m
[32m+[m[32m    case 0x77: printf("MOV    M,A"); break;[m
[32m+[m[32m    case 0x78: printf("MOV    A,B"); break;[m
[32m+[m[32m    case 0x79: printf("MOV    A,C"); break;[m
[32m+[m[32m    case 0x7a: printf("MOV    A,D"); break;[m
[32m+[m[32m    case 0x7b: printf("MOV    A,E"); break;[m
[32m+[m[32m    case 0x7c: printf("MOV    A,H"); break;[m
[32m+[m[32m    case 0x7d: printf("MOV    A,L"); break;[m
[32m+[m[32m    case 0x7e: printf("MOV    A,M"); break;[m
[32m+[m[32m    case 0x7f: printf("MOV    A,A"); break;[m
[32m+[m[41m      [m
[32m+[m[32m    case 0x80: printf("ADD    B"); break;[m
[32m+[m[32m    case 0x81: printf("ADD    C"); break;[m
[32m+[m[32m    case 0x82: printf("ADD    D"); break;[m
[32m+[m[32m    case 0x83: printf("ADD    E"); break;[m
[32m+[m[32m    case 0x84: printf("ADD    H"); break;[m
[32m+[m[32m    case 0x85: printf("ADD    L"); break;[m
[32m+[m[32m    case 0x86: printf("ADD    M"); break;[m
[32m+[m[32m    case 0x87: printf("ADD    A"); break;[m
[32m+[m[32m    case 0x88: printf("ADC    B"); break;[m
[32m+[m[32m    case 0x89: printf("ADC    C"); break;[m
[32m+[m[32m    case 0x8a: printf("ADC    D"); break;[m
[32m+[m[32m    case 0x8b: printf("ADC    E"); break;[m
[32m+[m[32m    case 0x8c: printf("ADC    H"); break;[m
[32m+[m[32m    case 0x8d: printf("ADC    L"); break;[m
[32m+[m[32m    case 0x8e: printf("ADC    M"); break;[m
[32m+[m[32m    case 0x8f: printf("ADC    A"); break;[m
[32m+[m[41m      [m
[32m+[m[32m    case 0x90: printf("SUB    B"); break;[m
[32m+[m[32m    case 0x91: printf("SUB    C"); break;[m
[32m+[m[32m    case 0x92: printf("SUB    D"); break;[m
[32m+[m[32m    case 0x93: printf("SUB    E"); break;[m
[32m+[m[32m    case 0x94: printf("SUB    H"); break;[m
[32m+[m[32m    case 0x95: printf("SUB    L"); break;[m
[32m+[m[32m    case 0x96: printf("SUB    M"); break;[m
[32m+[m[32m    case 0x97: printf("SUB    A"); break;[m
[32m+[m[32m    case 0x98: printf("SBB    B"); break;[m
[32m+[m[32m    case 0x99: printf("SBB    C"); break;[m
[32m+[m[32m    case 0x9a: printf("SBB    D"); break;[m
[32m+[m[32m    case 0x9b: printf("SBB    E"); break;[m
[32m+[m[32m    case 0x9c: printf("SBB    H"); break;[m
[32m+[m[32m    case 0x9d: printf("SBB    L"); break;[m
[32m+[m[32m    case 0x9e: printf("SBB    M"); break;[m
[32m+[m[32m    case 0x9f: printf("SBB    A"); break;[m
[32m+[m[41m      [m
[32m+[m[32m    case 0xa0: printf("ANA    B"); break;[m
[32m+[m[32m    case 0xa1: printf("ANA    C"); break;[m
[32m+[m[32m    case 0xa2: printf("ANA    D"); break;[m
[32m+[m[32m    case 0xa3: printf("ANA    E"); break;[m
[32m+[m[32m    case 0xa4: printf("ANA    H"); break;[m
[32m+[m[32m    case 0xa5: printf("ANA    L"); break;[m
[32m+[m[32m    case 0xa6: printf("ANA    M"); break;[m
[32m+[m[32m    case 0xa7: printf("ANA    A"); break;[m
[32m+[m[32m    case 0xa8: printf("XRA    B"); break;[m
[32m+[m[32m    case 0xa9: printf("XRA    C"); break;[m
[32m+[m[32m    case 0xaa: printf("XRA    D"); break;[m
[32m+[m[32m    case 0xab: printf("XRA    E"); break;[m
[32m+[m[32m    case 0xac: printf("XRA    H"); break;[m
[32m+[m[32m    case 0xad: printf("XRA    L"); break;[m
[32m+[m[32m    case 0xae: printf("XRA    M"); break;[m
[32m+[m[32m    case 0xaf: printf("XRA    A"); break;[m
[32m+[m[41m      [m
[32m+[m[32m    case 0xb0: printf("ORA    B"); break;[m
[32m+[m[32m    case 0xb1: printf("ORA    C"); b