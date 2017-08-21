/*
  File name : Utils.c

  Description : Misc helper utils for 8080 processor and ancilliary tools
  Author : Antony Toron
 */

#include "Utils.h"
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

unsigned char * readFileIntoBuffer(const char *filename, size_t *count) {
  FILE *f = fopen (filename, "rb");
  //printf ("Opening the file\n");

  if (f == NULL) {
    fprintf (stderr, "Couldn't open %s\n", filename);
    exit(1);
  }

  // get file size and read into buffer
  fseek(f, 0L, SEEK_END);
  int fsize = ftell(f);
  fseek(f, 0L, SEEK_SET);

  unsigned char *buffer = malloc(fsize * sizeof(char));

  size_t bytesRead = fread(buffer, 1, fsize, f);
  fclose(f);

  *count = bytesRead;

  return buffer;
}
