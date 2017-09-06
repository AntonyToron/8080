/*
  File name : Drivers.h
  
  Description : Drivers for all types of 8080 machines
  Author : Antony Toron

 */

#ifndef DRIVERS_H
#define DRIVERS_H

#include <stdint.h>
#include "CPU.h"

ArcadeMachine_T ArcadeMachine_INIT (ROM rom);

void ArcadeMachine_free (ArcadeMachine_T am_ports);

void INITIALIZE_IO (Drivers_T drivers, ROM rom, ArcadeMachine_T current_am);

void APPLY_DIP_SETTINGS (ArcadeMachine_T am, DIPSettings_T dip, ROM rom);

DIPSettings_T DIP_INIT ();

void DIP_SETTING_SET (DIPSettings_T dip, uint8_t bank, uint8_t which, uint8_t value);

uint8_t DIPS_Get (DIPSettings_T dip, uint8_t bank, uint8_t which);

uint16_t DIP_SETTINGS_DEBUG_GET (DIPSettings_T dip);

void DIP_SETTINGS_DEBUG_PRINT (DIPSettings_T dip);

void ARCADE_CONTROL (ArcadeMachine_T am, Control control, ROM rom);

uint8_t am_ports_get3(ArcadeMachine_T am_ports);

uint8_t am_ports_get5(ArcadeMachine_T am_ports);

#endif

