/*
  File name : arcade_machine_run.cpp
  Author : Antony Toron
  
  Description : Provides an interface to run an 8080 arcade machine from the
  terminal.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Types.h"
#include "arcade_machine.h"
extern "C" {
#include "Drivers.h"
}


int main (int argc, char **argv) {
  DIPSettings_T dip = DIP_INIT ();
  
  if (strcmp(argv[1], "invaders") == 0) {
    RUN_EMULATOR(INVADERS, dip);                          
  }
  else if (strcmp(argv[1], "ballbomb") == 0) {
    RUN_EMULATOR(BALLOON_BOMBER, dip);
  }
  else if (strcmp(argv[1], "gunfight") == 0) {
    RUN_EMULATOR(GUNFIGHT, dip);
  }
  else if (strcmp(argv[1], "encounters") == 0) {
    RUN_EMULATOR(SPACE_ENCOUNTERS, dip);
  }
  else if (strcmp(argv[1], "seawolf") == 0) {
    RUN_EMULATOR(SEAWOLF, dip);
  }
  else if (strcmp(argv[1], "m4") == 0) {
    RUN_EMULATOR(M4, dip);
  }
  else if (strcmp(argv[1], "bowling") == 0) {
    RUN_EMULATOR(BOWLING, dip);
  }
  else {
    fprintf (stderr, "This ROM is not supported\n");
    exit(1);
  }
  return 0;
}
