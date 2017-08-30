/*
  Run this script on an 8080 code snippet to see the assembly language
  translation.

  Author : Antony Toron
 */

#include "disassembler.h"
#include <stdio.h>

int main (int argc, char **argv) {
  char *fileName = argv[1];
  printf ("%s\n", fileName);
  
  decode8080CodeFile(fileName);

  printf ("Finished execution\n");
  return 0;
}
