/*
  File name : Utils.c

  Description : Misc helper utils for 8080 processor and ancilliary tools
  Author : Antony Toron
 */

#include "Utils.h"
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

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

// sound effects
Mix_Chunk *shot = NULL;
Mix_Chunk *invaderKilled = NULL;
Mix_Chunk *flash = NULL;

static void INIT_SDL () {
  if (SDL_Init(SDL_INIT_AUDIO) < 0) {
    printf ("Error in initializing sdl audio\n");
    exit(1);
  }

  if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) == -1) {
    printf ("Error in opening mix audio\n");
    exit(1);
  }

}

static void LOAD_WAVS () {
  shot = Mix_LoadWAV("./sound/3.wav");
  invaderKilled = Mix_LoadWAV("./sound/1.wav");
  flash = Mix_LoadWAV("./sound/2.wav");

  if ((shot == NULL) || (invaderKilled == NULL) || (flash == NULL)) {
    printf ("Error in loading wavs\n");
    exit(1);
  }
  
}


void playSoundEffect(const char *soundname) {
  if (strcmp(soundname, "shot") == 0) {
    // -1 tells mixer to look for first free channel
    if (Mix_PlayChannel(-1, shot, 0) == -1) {
      fprintf (stderr, "Could not play shoot.wav\n");
    }
  }
  else if (strcmp(soundname, "invaderkilled") == 0) {
    if (Mix_PlayChannel(-1, invaderKilled, 0) == -1) {
      fprintf (stderr, "Could not play invaderkilled.wav\n");
    }
  }
  else if (strcmp(soundname, "flash") == 0) {
    if (Mix_PlayChannel(-1, flash, 0) == -1) {
      fprintf (stderr, "Could not play flash.wav\n");
    }
  }
}

void free_sdl() {
  Mix_FreeChunk(shot);
  Mix_FreeChunk(invaderKilled);
  Mix_FreeChunk(flash);

  Mix_CloseAudio();

  SDL_Quit();
}


void INIT_AUDIO () {
  INIT_SDL();

  LOAD_WAVS();
}
