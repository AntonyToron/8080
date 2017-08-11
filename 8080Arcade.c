/*
  File name : 8080Arcade.c

  Description : Using the intel8080 processor as arcade machine
  Author : Antony Toron
 */

#include "CPU.h"
#include "Utils.h"
#include "Drivers.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

// implement drivers
Drivers_T DefaultDrivers () {
  Drivers_T drivers = Drivers_init();

  return drivers;
}

// -------------------------- ARCADE CONFIGS --------------------------

static ArcadeMachinePorts_T am_ports;

void ArcadeOut4 (uint8_t ac) {
  Arcade8080_write4 (ac, am_ports);
}

void ArcadeOut2 (uint8_t ac) {
  Arcade8080_write2 (ac, am_ports);
}

uint8_t ArcadeRead3 () {
  return Arcade8080_read3(am_ports);
}

Drivers_T ArcadeDrivers() {
  Drivers_T drivers = Drivers_init();
  am_ports = am_ports_init();

  // shift registers
  config_drivers_out_port(drivers, &ArcadeOut4, 4);
  config_drivers_out_port(drivers, &ArcadeOut2, 2);
  config_drivers_in_port(drivers, &ArcadeRead3, 3);
  
  return drivers;
}

// -------------------------- INVADERS STATE ----------------------------

void LOAD_ROM_invaders (State8080_T state) {
  // load into memory
  unsigned char *buffer = readFileIntoBuffer("invaders");

  // load the rom
  State8080_load_mem(state, 0, buffer);
    
  printf ("Successfully loaded ROM");
  
}

State8080_T INIT_STATE_invaders () {
  State8080_T state = State8080_init ();

  State8080_config_drivers_default(state, ArcadeDrivers());
  
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
