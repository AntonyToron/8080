/*
  File name : Drivers.h
  
  Description : Drivers for all types of 8080 machines
  Author : Antony Toron

 */

#include <stdint.h>
#include "arcade_machine.h"
#include "CPU.h"

enum Control {INSERT_COIN, REGISTER_COIN, P1_START_DOWN, P1_START_UP,
	      P2_START_DOWN, P2_START_UP, P1_SHOT_DOWN, P1_SHOT_UP,
	      P1_LEFT_DOWN, P1_RIGHT_DOWN, P1_LEFT_UP, P1_RIGHT_UP};

typedef enum Control Control;

typedef struct ArcadeMachine * ArcadeMachine_T;

ArcadeMachine_T ArcadeMachine_INIT (ROM rom);

void ArcadeMachine_free (ArcadeMachine_T am_ports);

void INITIALIZE_IO (Drivers_T drivers, ROM rom, ArcadeMachine_T current_am);

void DIP_SETTING_SET (ArcadeMachine_T am, int bank, int which, int value);

void ARCADE_CONTROL (ArcadeMachine_T am, Control control, ROM rom);

uint8_t am_ports_get3(ArcadeMachine_T am_ports);

uint8_t am_ports_get5(ArcadeMachine_T am_ports);

