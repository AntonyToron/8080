/*
  File name : Drivers.h
  
  Description : Drivers for all types of 8080 machines
  Author : Antony Toron

 */

#include <stdint.h>

typedef struct ArcadeMachinePorts * ArcadeMachinePorts_T;

ArcadeMachinePorts_T am_ports_init_invaders ();

void am_ports_free (ArcadeMachinePorts_T am_ports);

void Arcade8080_write4 (uint8_t ac, ArcadeMachinePorts_T am_ports);

void Arcade8080_write2 (uint8_t ac, ArcadeMachinePorts_T am_ports);

void Arcade8080_write3 (uint8_t ac, ArcadeMachinePorts_T am_ports);

void Arcade8080_write5 (uint8_t ac, ArcadeMachinePorts_T am_ports);

uint8_t Arcade8080_read3 (ArcadeMachinePorts_T am_ports);

uint8_t Arcade8080_read0 (ArcadeMachinePorts_T am_ports);

uint8_t Arcade8080_read1 (ArcadeMachinePorts_T am_ports);

uint8_t Arcade8080_read2 (ArcadeMachinePorts_T am_ports);


void INSERT_COIN (ArcadeMachinePorts_T am_ports);

void REGISTER_COIN (ArcadeMachinePorts_T am_ports);

void P1_START_DOWN (ArcadeMachinePorts_T am_ports);

void P1_START_UP (ArcadeMachinePorts_T am_ports);

void P2_START (ArcadeMachinePorts_T am_ports);

void P1_SHOT_DOWN (ArcadeMachinePorts_T am_ports);

void P1_SHOT_UP (ArcadeMachinePorts_T am_ports);

void P1_LEFT_DOWN (ArcadeMachinePorts_T am_ports);

void P1_RIGHT_DOWN (ArcadeMachinePorts_T am_ports);

void P1_LEFT_UP (ArcadeMachinePorts_T am_ports);

void P1_RIGHT_UP (ArcadeMachinePorts_T am_ports);


uint8_t am_ports_get3(ArcadeMachinePorts_T am_ports);

uint8_t am_ports_get5(ArcadeMachinePorts_T am_ports);

