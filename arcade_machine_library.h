/*
  File name : arcade_machine_library.h
  Author : Antony Toron

  Description : Defines an interface with starting the 8080 arcade machine.

 */

enum ROM {INVADERS, BALLOON_BOMBER};

void RUN_EMULATOR(ROM rom);
