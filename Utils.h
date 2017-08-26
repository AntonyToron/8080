/*
  File name : Utils.h

  Description : Header for Misc helper utils
  Author : Antony Toron
 */

#include <stddef.h>

unsigned char * readFileIntoBuffer(const char *filename, size_t * count);

void INIT_AUDIO ();

void playSoundEffect(const char *filename);

void free_sdl ();

