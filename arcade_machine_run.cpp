/*
  File name : arcade_machine_run.cpp
  Author : Antony Toron
  
  Description : Provides an interface to run an 8080 arcade machine from the
  terminal.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "arcade_machine.h"


int main (int argc, char **argv) {
  if (strcmp(argv[1], "invaders") == 0) {
    RUN_EMULATOR(INVADERS);                          
  }
  else if (strcmp(argv[1], "ballbomb") == 0) {
    RUN_EMULATOR(BALLOON_BOMBER);
  }
  else if (strcmp(argv[1], "gunfight") == 0) {
    RUN_EMULATOR(GUNFIGHT);
  }
  else if (strcmp(argv[1], "encounters") == 0) {
    RUN_EMULATOR(SPACE_ENCOUNTERS);
  }
  else if (strcmp(argv[1], "seawolf") == 0) {
    RUN_EMULATOR(SEAWOLF);
  }
  else if (strcmp(argv[1], "m4") == 0) {
    RUN_EMULATOR(M4);
  }
  else if (strcmp(argv[1], "bowling") == 0) {
    RUN_EMULATOR(BOWLING);
  }
  else {
    fprintf (stderr, "This ROM is not supported\n");
    exit(1);
  }
  return 0;
}
