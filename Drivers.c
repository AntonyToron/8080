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
  uint8_t port0;
  uint8_t port1;
  uint8_t port2;
  uint8_t port3;
  uint8_t port5;
  // fill in other necessary memory
};

ArcadeMachinePorts_T am_ports_init () {
  ArcadeMachinePorts_T am_ports = (struct ArcadeMachinePorts *) malloc (sizeof (struct ArcadeMachinePorts));

  am_ports->shift_registers[0] = 0;
  am_ports->shift_registers[1] = 0;
  am_ports->offset = 0;

  am_ports->port0 = 0x0E;
  am_ports->port1 = 0x08;
  am_ports->port2 = 0x00;
  am_ports->port3 = 0x00;
  am_ports->port5 = 0x00;
  
  return am_ports;
}

void am_ports_free (ArcadeMachinePorts_T am_ports) {
  free(am_ports);
}

// -------------------- SHIFT REGISTER ----------------------

// 8080 Arcade port 4: (Write), sets data in shift registers
// [8 bits a][8 bits b], shifts a into b, and the new value written to b
void Arcade8080_write4 (uint8_t ac, ArcadeMachinePorts_T am_ports) {
  am_ports->shift_registers[1] = am_ports->shift_registers[0];
  am_ports->shift_registers[0] = ac;
}

// 8080 Arcade port 2: (Write), sets the shift amount, DOESNT RETURN IT
// bits 0, 1, 2 set offset for 8 bit result
// i.e.
// xxxxxxxx:yyyyyyyy -> offset 2: 8 bit result = xxxxxxyy
// offset 7 : 8 bit result = xyyyyyyy
void Arcade8080_write2 (uint8_t ac, ArcadeMachinePorts_T am_ports) {
  am_ports->offset = ac;
}

// 8080 Arcade port 3: (Read) // reads data based on shift
// returns result from port 2 writes
uint8_t Arcade8080_read3 (ArcadeMachinePorts_T am_ports) {
  uint16_t result;

  result = (am_ports->shift_registers[0] << 8) | (am_ports->shift_registers[1]);
  result >>= (8 - am_ports->offset);

  return result & 0xff;
}

uint8_t Arcade8080_read0 (ArcadeMachinePorts_T am_ports) { 
  return am_ports->port0;
}

uint8_t Arcade8080_read1 (ArcadeMachinePorts_T am_ports) {
  return am_ports->port1;
}

uint8_t Arcade8080_read2 (ArcadeMachinePorts_T am_ports) {
  return am_ports->port2;
}

void Arcade8080_write3 (uint8_t ac, ArcadeMachinePorts_T am_ports) {
  /*
    bits:
    0 = SX0 0.raw // sound (loop, play until goes to 0)
    1 = SX1 1.raw // sound
    2 = SX2 2.raw // sound
    3 = SX3 3.raw // sound
    4 = SX4       // extended play
    5 = SX5       // AMP enable
    
   */
  
  am_ports->port3 = ac;
}

void Arcade8080_write5 (uint8_t ac, ArcadeMachinePorts_T am_ports) {
  /*
    bits:
    0 = SX6  4.raw // sound
    1 = SX7  5.raw // sound
    2 = SX8  6.raw // sound
    3 = SX9  7.raw // sound
    4 = SX10 8.raw // sound
   
   */

  am_ports->port5 = ac;
}

// --------------------- AUXILIARY FUNCTIONS ------------------- //


void INSERT_COIN (ArcadeMachinePorts_T am_ports) {
  am_ports->port1 |= 0x01;
}
void REGISTER_COIN (ArcadeMachinePorts_T am_ports) {
  am_ports->port1 &= ~0x01;
}

void P1_START_DOWN (ArcadeMachinePorts_T am_ports) {
  am_ports->port1 |= 0x04;
}

void P1_START_UP (ArcadeMachinePorts_T am_ports) {
  am_ports->port1 &= ~0x04;
}

void P2_START (ArcadeMachinePorts_T am_ports) {
  am_ports->port1 |= 0x02;
}

void P1_SHOT_DOWN (ArcadeMachinePorts_T am_ports) {
  am_ports->port1 |= 0x10;
}

void P1_SHOT_UP (ArcadeMachinePorts_T am_ports) {
  am_ports->port1 &= ~0x10;
}

void P1_LEFT_DOWN (ArcadeMachinePorts_T am_ports) {
  am_ports->port1 |= 0x20;
}

void P1_RIGHT_DOWN (ArcadeMachinePorts_T am_ports) {
  am_ports->port1 |= 0x40;
}

void P1_LEFT_UP (ArcadeMachinePorts_T am_ports) {
  am_ports->port1 &= ~0x20;
}

void P1_RIGHT_UP (ArcadeMachinePorts_T am_ports) {
  am_ports->port1 &= ~0x40;
}

uint8_t am_ports_get3(ArcadeMachinePorts_T am_ports) {
  return am_ports->port3;
}

uint8_t am_ports_get5(ArcadeMachinePorts_T am_ports) {
  return am_ports->port5;
}


