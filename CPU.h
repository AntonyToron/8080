/*
  File name : CPU.h
  
  Description : Header file for 8080 processor.
  Author : Antony Toron
 */

#include <stdint.h>

typedef struct State8080* State8080_T;
typedef struct ConditionCodes* ConditionCodes_T;
typedef struct Drivers* Drivers_T;

void Emulate8080Op(State8080_T state);

State8080_T State8080_init();

ConditionCodes_T ConditionCodes_init();

Drivers_T Drivers_init();

void State8080_load_mem(State8080_T state, int start, unsigned char *buffer);

void State8080_config_drivers_in_port(State8080_T state, uint8_t (*in) (), uint8_t port);

void State8080_config_drivers_out_port(State8080_T state, void (*out) (uint8_t), uint8_t port);

void State8080_config_drivers_default(State8080_T state, Drivers_T drivers);
