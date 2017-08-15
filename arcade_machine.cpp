/*
  File name: arcade_machine.cpp

  Description: Platform layer, defines the window and callbacks to use 
  the 8080 processor.

  Author: Antony Toron
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <ctime>
#include <iostream>
extern "C" {
  #include "CPU.h"
}
extern "C" {
  #include "Utils.h"
}
extern "C" {
  #include "Drivers.h"
}

#include <string.h>
#include <thread>
#include <GL/glut.h>

const int RGB_ON = 0xFFFFFFFF;
const int RGB_OFF = 0x00000000;

const unsigned int W = 224;
const unsigned int H = 256;
double clock_time_miliseconds = 1000.0 / 2000000.0;

State8080_T state = NULL;
bool CPU_on = false;
bool hardware_interrupt = false;

uint8_t last_interrupt = 0;

// DISPLAY
unsigned char windowPixels[W * H * 4];


// -------------------------- STATE INITIALIZATION ---------------------

// implement drivers
Drivers_T DefaultDrivers () {
  Drivers_T drivers = Drivers_init();

  return drivers;
}

// -------------------------- ARCADE CONFIGS --------------------------

static ArcadeMachinePorts_T am_ports;

void ArcadeOut4 (uint8_t ac) {
  Arcade8080_write4 (ac, am_ports);
}

void ArcadeOut2 (uint8_t ac) {
  Arcade8080_write2 (ac, am_ports);
}

uint8_t ArcadeRead3 () {
  return Arcade8080_read3(am_ports);
}

Drivers_T ArcadeDrivers() {
  Drivers_T drivers = Drivers_init();
  am_ports = am_ports_init();

  // shift registers
  config_drivers_out_port(drivers, &ArcadeOut4, 4);
  config_drivers_out_port(drivers, &ArcadeOut2, 2);
  config_drivers_in_port(drivers, &ArcadeRead3, 3);
  
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

  State8080_config_drivers_default(state, ArcadeDrivers());
  
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
  int i, j;
  
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  //glBegin(GL_TRIANGLES);
  //glVertex3f(-0.5, -0.5, 0.0);
  //glVertex3f(0.5, 0.0, 0.0);
  //glVertex3f(0.0, 0.5, 0.0);
  //glEnd();
  //glRasterPos2i(0,0);
  
  // 1-bit space invaders video into window
  // ASSUME SPACE INVADERS FOR NOW
  uint16_t startVideoMemory = 0x2400;
  uint8_t *videoMemory = pointerToMemoryAt(state, startVideoMemory);

  // copy into window
  for (i = 0; i < 224; i++) {
    for (j = 0; j < 256; j+=8) {
      int p;
      // Read first 1-bit pixel, divide by 8 bc 8 pixels in byte
      unsigned char pix = videoMemory[(i * (256/8)) + j/8];

      // 8 output vertical pixels --> vertical flip, j start at last line
      int offset = (255-j)*(224*4) + (i*4);
      unsigned int *p1 = (unsigned int *)(&windowPixels[offset]);
      for (p = 0; p < 8; p++) {
	if (0 != (pix & (1 << p))) { // white pixel
	  //windowPixels[offset][0] = 255;
	  //windowPixels[offset][1] = 255;
	  //windowPixels[offset][2] = 255;
	  *p1 = RGB_ON;
	  
	  fflush(stdout);
	}
	else
	  *p1 = RGB_OFF;
	p1 -= 224;
	
      }
    }
  }
  
  glRasterPos2f(-1, 1); // IMPORTANT FOR STARTING AT 0, 0
  glPixelZoom(1 * 2, -1 * 2);   // MIRROR IMAGE, and ZOOM IN, to window size
  
  glDrawPixels(W, H, GL_RGBA, GL_UNSIGNED_BYTE, windowPixels);

  
  
  
  glutSwapBuffers();
  
  /*
  glGenTextures (1, &texID);
  glBindTextures (GL_TEXTURE_RECTANGLE_EXT, texID);
  glTexImage2D (GL_TEXTURE_RECTANGLE_EXT, 0, GL_RGBA, 640, 480, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, windowPixels);
  glBegin (GL_QUADS);
  glTexCoord2f (0, 0);
  glVertex2f (0, 0);
  glTexCoord2f (W, 0);
  glVertex2f (W, 0);
  glTexCoord2f (640, 480);
  glVertex2f (W, H);
  glTexCoord2f (0, 480);
  glVertex2f (0, H);
  glEnd();
  */
}

void changeSize(int w, int h) {
  double x = w / W;
  double y = h / H;
  
  
  //glPixelZoom(x, y);   // MIRROR IMAGE
  //glRasterPos2f(-1, 1); // IMPORTANT FOR STARTING AT 0, 0
  
  
  
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

void graphicsInterrupt() {
  // set IE to true
  //unsigned char IE = 0xFB;
  //Emulate8080Op(state, &IE);

  State8080_setIE(state, 1);

  // say that it was from hardware
  hardware_interrupt = true;

  if (last_interrupt == 1) {
    State8080_pushInterrupt(state, 2); // graphics interrupt
    last_interrupt = 2;
  }
  else {
    State8080_pushInterrupt(state, 1);
    last_interrupt = 1;
  }
}

void graphicsThread(int argc, char **argv) {
  // init GLUT and create window
  glutInit(&argc, argv);

  glutInitWindowPosition(100, 100);
  glutInitWindowSize(W * 2, H * 2);
  glutInitDisplayMode(GLUT_RGBA | GLUT_SINGLE); // double?
  glutCreateWindow("Arcade Machine 8080");
  
  // register callbacks
  glutDisplayFunc(render);
  glutIdleFunc(render);
  //glutReshapeFunc(changeSize);

  // external entries callbacks
  glutKeyboardFunc (processNormalKeys);
  glutSpecialFunc(processSpecialKeys);

  // enter GLUT event processing cycle
  glutMainLoop();
}

void hardwareThread() {
  printf ("Hello from hardware thread\n");
  
  // --------------- arcade specific --------------------- //
  // add timer for 1/60 seconds to process graphics, 16.67 milliseconds
  for (;;) {
    graphicsInterrupt();

        
    usleep(16670);
  }
}

void processorThread() {
  printf("hello from processor\n");
  fflush(stdout);

  while (CPU_on) {
    // check if interrupts enabled, and an interrupt happened
    if (State8080_ie(state) && hardware_interrupt) { 
      hardware_interrupt = false;
      
      printf ("Got an interrupt");
      
      // get interrupt set by hardware  11AAA111
      unsigned char op = (State8080_popInterrupt(state) << 3) | 0xC7;

      Emulate8080Op(state, &op);

      //unsigned char DI = 0xF3;
      
      // DI
      //Emulate8080Op(state, &DI);
      State8080_setIE(state, 1);

      fflush(stdout);
    }
    else {
      int cycles = op_clockCycles(state); // how many cycles this should take
      // start timer
      std::clock_t start;
      start = std::clock();
      
      Emulate8080State(state);
      fflush(stdout);

      double instructionTime = (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000);
      //std::cout << "Time: " << instructionTime << "ms" << std::endl;
      
      
      // check if instruction took enough time
      double targetTime = (cycles * 1.0) * clock_time_miliseconds;
      if (instructionTime < targetTime) {
	double waitTime = targetTime - instructionTime;
	//std::cout << "sleeping for " << waitTime << "ms" << std::endl;
	usleep(waitTime * 1000); // sleep in microseconds
      }

      //std::cout << "Should have taken " << targetTime << "ms" << std::endl;
      //std::cout << "Now it took: " << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000) << " ms" << std::endl;

      fflush(stdout);
     
    }
  } 
}


int main (int argc, char **argv) {
  INITIALIZE_PROCESSOR(state, argv);

  CPU_on = true;
  
  std::thread graphics (graphicsThread, argc, argv); // graphics thread
  std::thread cpu (processorThread); // processor thread
  std::thread hardware (hardwareThread); // hardware thread

  // synchronize threads
  graphics.join();
  cpu.join();
  hardware.join();
  
  return 0;
}

