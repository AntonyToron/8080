/*
  File name : cpu_test.c
  Author : Antony Toron

  Description: Designed at testing all instructions of the 8080 CPU.

 */

#include "Types.h"
#include "CPU.h"
#include "Utils.h"
#include "arcade_machine.h"
#include "Drivers.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

Drivers_T DefaultDrivers () {
  Drivers_T drivers = Drivers_init();

  return drivers;
}

// -------------------------- ARCADE CONFIGS --------------------------

static ArcadeMachine_T am;

void Out5 (uint8_t ac) {
  
}

Drivers_T ArcadeDrivers() {
  Drivers_T drivers = Drivers_init();
  am = ArcadeMachine_INIT(INVADERS);

  // shift registers
  config_drivers_out_port(drivers, &Out5, 5);
  
  
  return drivers;
}

// ----------------------------- TEST STATE ----------------------------

void LOAD_test (State8080_T state) {
  // load into memory
  size_t bytesRead;
  unsigned char *buffer = readFileIntoBuffer("cpudiag.bin", &bytesRead);

  unsigned char jump[3] = {0xc3, 0x00, 0x01};
  State8080_load_mem(state, 0x000, (size_t) 3, jump);
  unsigned char x[1] = {0x01};
  State8080_load_mem(state, 0x0002, 0x02 + (size_t) 1, x);

  unsigned char stack[1] = {0x70};
  State8080_load_mem(state, 0x368, 0x368 + (size_t) 1, stack);

  // load the rom
  State8080_load_mem(state, 0x100, (size_t) 0x100 + bytesRead, buffer);
    
  printf ("Successfully loaded ROM");
  
}

State8080_T INIT_STATE_test () {
  State8080_T state = State8080_init ();

  State8080_config_drivers_default(state, ArcadeDrivers());
  
  return state;
}

//------------------------------------------------------------------------

int main(int argc, char **argv) {
  State8080_T state;
  state = INIT_STATE_test ();
  LOAD_test (state);


  

  // BEGIN EMULATION
  while (1) {
    
    Emulate8080State(state);
  }

  return 0;
}
