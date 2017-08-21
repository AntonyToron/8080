/*
  File name : CPU.h
  
  Description : Header file for 8080 processor.
  Author : Antony Toron
 */

#include <stdint.h>
#include <stddef.h>

typedef struct State8080* State8080_T;
typedef struct ConditionCodes* ConditionCodes_T;
typedef struct Drivers* Drivers_T;

void Emulate8080Op(State8080_T state, unsigned char *opcode);

void Emulate8080State(State8080_T state);

State8080_T State8080_init();

void State8080_free (State8080_T state);

ConditionCodes_T ConditionCodes_init();

Drivers_T Drivers_init();

void State8080_load_mem(State8080_T state, int start, size_t end, unsigned char *buffer);

void State8080_config_drivers_in_port(State8080_T state, uint8_t (*in) (), uint8_t port);

void State8080_config_drivers_out_port(State8080_T state, void (*out) (uint8_t), uint8_t port);

void State8080_config_drivers_default(State8080_T state, Drivers_T drivers);

void config_drivers_in_port(Drivers_T drivers, uint8_t (*in) (), uint8_t port);

void config_drivers_out_port(Drivers_T drivers, void (*out) (uint8_t), uint8_t port);

uint8_t State8080_ie(State8080_T state);

void State8080_setIE(State8080_T state, uint8_t val);

void State8080_pushInterrupt(State8080_T state, uint8_t int_num);

uint8_t State8080_popInterrupt(State8080_T state);

int op_clockCycles(State8080_T state);

uint8_t *pointerToMemoryAt(State8080_T state, uint16_t addr);

void startPrintingOut();
