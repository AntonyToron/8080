/*
  File name : Drivers.c
  
  Description : Returns functions to perform for ports, contains all
         information about ports
  Author : Antony Toron
*/

#include "Drivers.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// -------------------- ARCADE MACHINE ---------------------

struct ArcadeMachinePorts {
  uint8_t shift_registers[2];
  uint8_t offset;
  
  // fill in other necessary memory
};

ArcadeMachinePorts_T am_ports_init () {
  ArcadeMachinePorts_T am_ports = (struct ArcadeMachinePorts *) malloc (sizeof (struct ArcadeMachinePorts));

  am_ports->shift_registers[0] = 0;
  am_ports->shift_registers[1] = 0;
  am_ports->offset = 0;
  
  return am_ports;
}

// -------------------- SHIFT REGISTER ----------------------

// 8080 Arcade port 4: (Write), sets data in shift registers
// [8 bits a][8 bits b], shifts a into b, and the new value written to b
void Arcade8080_write4 (uint8_t ac, ArcadeMachinePorts_T am_ports) {
  am_ports->shift_registers[1] = am_ports->shift_registers[0];
  am_ports->shift_registers[1] = ac;
}

// 8080 Arcade port 2: (Write), sets the shift amount, DOESNT RETURN IT
// bits 0, 1, 2 set offset for 8 bit result
// i.e.
// xxxxxxxx:yyyyyyyy -> offset 2: 8 bit result = xxxxxxyy
// offset 7 : 8 bit result = xyyyyyyy
void Arcade8080_write2 (uint8_t ac, ArcadeMachinePorts_T am_ports) {
  am_ports->offset = 0;
}

// 8080 Arcade port 3: (Read) // reads data based on shift
// returns result from port 2 writes
uint8_t Arcade8080_read3 (ArcadeMachinePorts_T am_ports) {
  uint8_t result = 0;

  

  return result;
}


