/*
  File name: arcade_machine.cpp

  Description: Platform layer, defines the window and callbacks to use 
  the 8080 processor.

  Author: Antony Toron
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
extern "C" {
  #include "CPU.h"
}
extern "C" {
  #include "Utils.h"
}

#include <string.h>
#include <thread>
#include <GL/glut.h>

const unsigned int W = 200;
const unsigned int H = 200;

State8080_T state = NULL;
bool CPU_on = false;

// -------------------------- STATE INITIALIZATION ---------------------

// implement drivers
Drivers_T ArcadeDrivers () {
  Drivers_T drivers = Drivers_init();

  return drivers;
}

// -------------------------- INVADERS STATE ----------------------------

void LOAD_ROM_invaders (State8080_T state) {
  // load into memory
  unsigned char *buffer = readFileIntoBuffer("invaders");

  // load the rom
  State8080_load_mem(state, 0, buffer);
    
  printf ("Successfully loaded ROM\n");
  
}

State8080_T INIT_STATE_invaders () {
  state = State8080_init ();
  
  return state;
}

void INITIALIZE_PROCESSOR(State8080_T state, char **argv) {
  if (strcmp(argv[1], "invaders") == 0) {
    state = INIT_STATE_invaders ();

    LOAD_ROM_invaders (state);                          
  }
  else {
    fprintf (stderr, "This ROM is not supported");
    exit(1);
  }
}





// ----------------------- MACHINE INFORMATION --------------------

void render() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glBegin(GL_TRIANGLES);
  glVertex3f(-0.5, -0.5, 0.0);
  glVertex3f(0.5, 0.0, 0.0);
  glVertex3f(0.0, 0.5, 0.0);
  glEnd();

  glutSwapBuffers();
}

void processNormalKeys(unsigned char key, int x, int y) {
  if (key == 27) // escape
    CPU_on = false; // turn off CPU
    exit(0);
  // can config a, w, s, d too
}

void processSpecialKeys(int key, int x, int y) {
  switch (key) {
  case GLUT_KEY_LEFT:
    printf ("Left");
    break;
  case GLUT_KEY_RIGHT:
    printf ("Right");
    break;
  case GLUT_KEY_UP:
    printf("Up");
    break;
  case GLUT_KEY_DOWN:
    printf("Down");
    break;
  }

  fflush(stdout);
}

void hardwareThread(int argc, char **argv) {
  // init GLUT and create window
  glutInit(&argc, argv);

  glutInitWindowPosition(100, 100);
  glutInitWindowSize(W, H);
  glutInitDisplayMode(GLUT_RGBA | GLUT_SINGLE); // double?
  glutCreateWindow("Arcade Machine 8080");
  
  // register callbacks
  glutDisplayFunc(render);
  //glutIdleFunc(render);

  // external entries callbacks
  glutKeyboardFunc (processNormalKeys);
  glutSpecialFunc(processSpecialKeys);
  

  // enter GLUT event processing cycle
  glutMainLoop();
}

void processorThread() {
  printf("hello from processor\n");
  fflush(stdout);

  while (CPU_on) {
    if (State8080_ie(state)) { // check if interrupts enabled

      printf ("Got an interrupt");
      
      // get interrupt set by hardware
      unsigned char op = State8080_popInterrupt(state) << 3; // 00AAA000
      Emulate8080Op(state, &op);

      unsigned char DI = 0xF3;
      
      // DI
      Emulate8080Op(state, &DI);

      fflush(stdout);
    }
    else {
      Emulate8080State(state);
      fflush(stdout);
    }
  }
  
}


int main (int argc, char **argv) {
  INITIALIZE_PROCESSOR(state, argv);

  CPU_on = true;
  
  std::thread graphics (hardwareThread, argc, argv); // graphics thread
  std::thread cpu (processorThread); // processor thread

  // synchronize threads
  graphics.join();
  cpu.join();
  
  return 0;
}

