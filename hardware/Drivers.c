/*
  File name : Drivers.c
  
  Description : Returns functions to perform for ports, contains all
         information about ports
  Author : Antony Toron
*/

#include "Types.h"
#include "Drivers.h"
#include "arcade_machine.h"
#include "CPU.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// -------------------- ARCADE MACHINE ---------------------

struct DIPSettings {
  //uint8_t bank_1; // 1 = ON, 0 = OFF
  //uint8_t bank_2;
  uint8_t bank_1[8];
  uint8_t bank_2[8];
};

struct ArcadeMachine {
  uint8_t shift_registers[2];
  uint8_t offset;
  uint8_t port0;
  uint8_t port1;
  uint8_t port2;
  uint8_t port3;
  uint8_t port5;

  DIPSettings_T dip;
  // fill in other necessary memory
};

static ArcadeMachine_T CURRENT_AM = NULL; // reference to the functions below

ArcadeMachine_T ArcadeMachine_INIT (ROM rom) {
  ArcadeMachine_T am = (struct ArcadeMachine *) malloc (sizeof (struct ArcadeMachine));

  switch (rom) {
  case INVADERS:
    am->shift_registers[0] = 0;
    am->shift_registers[1] = 0;
    am->offset = 0;

    am->port0 = 0x0E;
    am->port1 = 0x08;
    am->port2 = 0x00;
    am->port3 = 0x00;
    am->port5 = 0x00;
    break;
  case GUNFIGHT:
    am->shift_registers[0] = 0;
    am->shift_registers[1] = 0;
    am->offset = 0;
    
    am->port0 = 0x00;
    am->port1 = 0x00;
    am->port2 = 0x00;
    am->port3 = 0x00;
    am->port5 = 0x00;
    break;
  }
  
  return am;
}

void APPLY_DIP_SETTINGS (ArcadeMachine_T am, DIPSettings_T dip, ROM rom) {
  
  // AS OF NOW, DON'T NEED TO ASSIGN THIS, CAN ASSIGN A COPY LATER (NOT THE
  // ORIGINAL POINTER SINCE THE REFERENCES TO IT WILL MESS UP IN EMULATOR)
  //am->dip = dip;

  // change read ports based on individual ROMS
  switch (rom) {
  case INVADERS:
    printf ("Initializing DIPS for INVADERS\n");

    // bits 0-1 = dips 1-2
    am->port2 |= ((dip->bank_1[1 - 1] << 1) | (dip->bank_1[2 - 1]));
    // bit 3 = dip 6
    am->port2 |= (dip->bank_1[6 - 1] << 3); 
    // bit 7 = dip 4
    am->port2 |= (dip->bank_1[4 - 1] << 7); 
    
    break;
  }
}

void DIP_SETTING_SET (DIPSettings_T dip, uint8_t bank, uint8_t which, uint8_t value) {
  if (bank == 1) {
    //dip->bank_1 &= (value << which);
    dip->bank_1[which] = value;
  }
  else if (bank == 2) {
    //dip->bank_2 &= (value << which);
    dip->bank_2[which] = value;
  }
}

uint16_t DIP_SETTINGS_DEBUG_GET (DIPSettings_T dip) {
  //printf ("debug : %i, %i\n", dip->bank_1, dip->bank_2);
  //return (dip->bank_1 << 8) | (dip->bank_2);
  uint16_t settings = 0;
  for (int i = 0; i < 8; i++) {
    /*if (dip->bank_1[i] != 0) {
      printf ("not 0 : %i\n", i);
    }
    if (dip->bank_2[i] != 0) {
      printf ("not 0 (2) : %i\n", i);
      }*/
    settings &= dip->bank_1[i] << (15 - i);
    settings &= dip->bank_2[i] << (7 - i);
  }
  return settings;
}

void DIP_SETTINGS_DEBUG_PRINT (DIPSettings_T dip) {
  printf ("Bank 1: ");
  for (int i = 0; i < 8; i++) {
    printf ("%i", dip->bank_1[i]);
  }
  printf ("\nBank 2: ");
  for (int i = 0; i < 8; i++) {
    printf ("%i", dip->bank_2[i]);
  }
  printf ("\n");
  fflush(stdout);
}

DIPSettings_T DIP_INIT () {
  DIPSettings_T dip = (struct DIPSettings *) malloc (sizeof (struct DIPSettings));

  for (int i = 0; i < 8; i++) {
    dip->bank_1[i] = 0;
    dip->bank_2[i] = 0;
  }
  
  return dip;
}

void ArcadeMachine_free (ArcadeMachine_T am) {
  if (am->dip != NULL) {
    //free(am->dip);
  }
  
  free(am);
}

// --------------------- INVADERS IO -------------------------- //

// -------------------- SHIFT REGISTER ----------------------

// 8080 Arcade port 4: (Write), sets data in shift registers
// [8 bits a][8 bits b], shifts a into b, and the new value written to b
void invaders_out4 (uint8_t ac, ArcadeMachine_T am_ports) {
  am_ports->shift_registers[1] = am_ports->shift_registers[0];
  am_ports->shift_registers[0] = ac;
}

// 8080 Arcade port 2: (Write), sets the shift amount, DOESNT RETURN IT
// bits 0, 1, 2 set offset for 8 bit result
// i.e.
// xxxxxxxx:yyyyyyyy -> offset 2: 8 bit result = xxxxxxyy
// offset 7 : 8 bit result = xyyyyyyy
void invaders_out2 (uint8_t ac, ArcadeMachine_T am_ports) {
  am_ports->offset = ac;
}

// 8080 Arcade port 3: (Read) // reads data based on shift
// returns result from port 2 writes
uint8_t invaders_in3 (ArcadeMachine_T am_ports) {
  uint16_t result;

  result = (am_ports->shift_registers[0] << 8) | (am_ports->shift_registers[1]);
  result >>= (8 - am_ports->offset);

  return result & 0xff;
}

uint8_t invaders_in0 (ArcadeMachine_T am_ports) { 
  return am_ports->port0;
}

uint8_t invaders_in1 (ArcadeMachine_T am_ports) {
  return am_ports->port1;
}

uint8_t invaders_in2 (ArcadeMachine_T am_ports) {
  return am_ports->port2;
}

void invaders_out3 (uint8_t ac, ArcadeMachine_T am_ports) {
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

void invaders_out5 (uint8_t ac, ArcadeMachine_T am_ports) {
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

void invaders_write4 (uint8_t ac) {
  invaders_out4(ac, CURRENT_AM);
}

void invaders_write2 (uint8_t ac) {
  invaders_out2(ac, CURRENT_AM);
}

uint8_t invaders_read3 () {
  return invaders_in3(CURRENT_AM);
}

uint8_t invaders_read0 () { 
  return invaders_in0(CURRENT_AM);
}

uint8_t invaders_read1 () {
  return invaders_in1(CURRENT_AM);
}

uint8_t invaders_read2 () {
  return invaders_in2(CURRENT_AM);
}

void invaders_write3 (uint8_t ac) {
  invaders_out3(ac, CURRENT_AM);
}

void invaders_write5 (uint8_t ac) {
  invaders_out5(ac, CURRENT_AM);
}

// ----------------------- END INVADERS IO --------------------- //

// ---------------------- DRIVER INITIALIZATION ---------------- //

void INITIALIZE_IO (Drivers_T drivers, ROM rom, ArcadeMachine_T am) {
  CURRENT_AM = am;
  
  switch (rom) {
  case INVADERS:
    // OUT
    config_drivers_out_port(drivers, &invaders_write4, 4); // shift registers
    config_drivers_out_port(drivers, &invaders_write2, 2);
    config_drivers_out_port(drivers, &invaders_write3, 3); // sound
    config_drivers_out_port(drivers, &invaders_write5, 5);
    
    // IN
    config_drivers_in_port(drivers, &invaders_read3, 3);
    config_drivers_in_port(drivers, &invaders_read0, 0);
    config_drivers_in_port(drivers, &invaders_read1, 1);
    config_drivers_in_port(drivers, &invaders_read2, 2);
    break;
  default:
    printf ("Not performing any specific IO hookup\n");
  }
}

// --------------------- AUXILIARY FUNCTIONS ------------------- //

void ARCADE_CONTROL(ArcadeMachine_T am, Control control, ROM rom) {
  switch (rom) {
  case INVADERS:
    switch (control) {
    case INSERT_COIN:
      am->port1 |= 0x01;
      break;
    case REGISTER_COIN:
      am->port1 &= ~0x01;
      break;
    case P1_START_DOWN:
      am->port1 |= 0x04;
      break;
    case P1_START_UP:
      am->port1 &= ~0x04;
      break;
    case P2_START_DOWN:
      am->port1 |= 0x02;
      break;
    case P2_START_UP:
      am->port1 &= ~0x02;
      break;
    case P1_SHOT_DOWN:
      am->port1 |= 0x10;
      break;
    case P1_SHOT_UP:
      am->port1 &= ~0x10;
      break;
    case P1_LEFT_DOWN:
      am->port1 |= 0x20;
      break;
    case P1_RIGHT_DOWN:
      am->port1 |= 0x40;
      break;
    case P1_LEFT_UP:
      am->port1 &= ~0x20;
      break;
    case P1_RIGHT_UP:
      am->port1 &= ~0x40;
      break;
    default:
      printf ("Undefined control for Invaders\n");
      break;
    }
    break;
  default:
    printf ("Don't know how to handle controls for this ROM\n");
  }
}

uint8_t am_ports_get3(ArcadeMachine_T am_ports) {
  return am_ports->port3;
}

uint8_t am_ports_get5(ArcadeMachine_T am_ports) {
  return am_ports->port5;
}


