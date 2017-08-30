/*
  File name : arcade_machine.h
  Author : Antony Toron

  Description : Defines an interface with starting the 8080 arcade machine.

 */

#ifndef ARCADE_MACHINE_H
#define ARCADE_MACHINE_H

enum ROM {INVADERS, BALLOON_BOMBER, GUNFIGHT, SPACE_ENCOUNTERS, SEAWOLF, M4,
          BOWLING};

typedef enum ROM ROM;

void RUN_EMULATOR(ROM rom);

#endif
