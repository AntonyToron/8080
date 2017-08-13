/*
  File name : Drivers.h
  
  Description : Drivers for all types of 8080 machines
  Author : Antony Toron

 */

#include <stdint.h>

typedef struct ArcadeMachinePorts * ArcadeMachinePorts_T;

ArcadeMachinePorts_T am_ports_init ();

void Arcade8080_write4 (uint8_t ac, ArcadeMachinePorts_T am_ports);

void Arcade8080_write2 (uint8_t ac, ArcadeMachinePorts_T am_ports);

uint8_t Arcade8080_read3 (ArcadeMachinePorts_T am_ports);
