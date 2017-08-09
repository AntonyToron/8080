/*
  File name : 8080Arcade.c

  Description : Using the intel8080 processor as arcade machine
  Author : Antony Toron
 */

#include "CPU.h"
#include "Utils.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

// implement drivers
Drivers_T ArcadeDrivers () {
  Drivers_T drivers = Drivers_init();

  return drivers;
}

// -------------------------- INVADERS STATE ----------------------------

void LOAD_ROM_invaders (State8080_T state) {
  // load into memory
  unsigned char *buffer = readFileIntoBuffer("invaders");

  // load the rom
  for (int i = 0; i < 2048; i++) {
    state->memory[i] = (uint8_t) *buffer;
    buffer++;
  }
  
  printf ("Successfully loaded ROM");
  
}

State8080_T INIT_STATE_invaders () {
  State8080_T state = State8080_init ();
  
  return state;
}

//------------------------------------------------------------------------

int main(int argc, char **argv) {
  State8080_T state;
  
  if (strcmp(argv[1], "invaders") == 0) {
    state = INIT_STATE_invaders ();

    LOAD_ROM_invaders (state);                          
  }
  else {
    fprintf (stderr, "This ROM is not supported");
    exit(1);
  }

  // BEGIN EMULATION
  while (1) {
    
    Emulate8080Op(state);
  }
}
