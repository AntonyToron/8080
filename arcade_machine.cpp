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
#include <signal.h>
#include <mutex>
#include <condition_variable>

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
unsigned int CURRENT_WIDTH = W * 2;
unsigned int CURRENT_HEIGHT = H * 2;

double clock_time_milliseconds = 1000.0 / 2000000.0;

State8080_T state = NULL;
bool CPU_on = false;
bool hardware_interrupt = false;

uint8_t last_interrupt = 0;

// DISPLAY
unsigned char windowPixels[W * H * 4];


std::mutex m;
std::condition_variable cv;

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

uint8_t ArcadeRead0 () {
  return Arcade8080_read0(am_ports);
}

Drivers_T ArcadeDrivers() {
  Drivers_T drivers = Drivers_init();
  am_ports = am_ports_init();

  // shift registers
  config_drivers_out_port(drivers, &ArcadeOut4, 4);
  config_drivers_out_port(drivers, &ArcadeOut2, 2);
  config_drivers_in_port(drivers, &ArcadeRead3, 3);
  config_drivers_in_port(drivers, &ArcadeRead0, 0);
  
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

void populateWindowFromMemory() {
  int i, j;
  
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
	  *p1 = RGB_ON;
	  
	  fflush(stdout);
	}
	else
	  *p1 = RGB_OFF;
	p1 -= 224;
	
      }
    }
  }
}

void render() {
  // ADDS A LOCK ON RENDERING (THIS IS THE IDLE FUNC, SO IT LOCKS UP AND ONLY
  // RENDERS WHEN THE 60Hz WINDOW IS UP)
  std::unique_lock<std::mutex> lk(m);
  cv.wait_for(lk, std::chrono::microseconds(16670), []{return hardware_interrupt;});
  // RENDER SCREEN
  
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  glRasterPos2f(-1, 1); // IMPORTANT FOR STARTING AT 0, 0
  glPixelZoom(1 * 2, -1 * 2);   // MIRROR IMAGE, and ZOOM IN, to window size
  
  glDrawPixels(W, H, GL_RGBA, GL_UNSIGNED_BYTE, windowPixels);
  
  glutSwapBuffers();
  
}

void renderCallback(int x) {
  render();
}


void changeSize(int w, int h) {
  double x = w / W;
  double y = h / H;
  
  
  //glPixelZoom(x, y);   // MIRROR IMAGE
  //glRasterPos2f(-1, 1); // IMPORTANT FOR STARTING AT 0, 0

  CURRENT_WIDTH = w;
  CURRENT_HEIGHT = h;
  
  
}

void processNormalKeys(unsigned char key, int x, int y) {
  if (key == 27) {// escape
    CPU_on = false; // turn off CPU
    State8080_free(state);
    exit(0);
  }
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

void graphicsInterrupt(int value) {
  // set IE to true
  //unsigned char IE = 0xFB;
  //Emulate8080Op(state, &IE);

  // PERFORM THE GRAPHICS INTERRUPT
  
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

  cv.notify_all(); // notify of hardware interrupt
  
  // POPULATE WINDOW WITH IN-MEMORY VIDEO RAM, (effective 60hz draw)
  populateWindowFromMemory();
}

void graphicsInterruptCallback(int x) {
  graphicsInterrupt(0);
  
  glutPostRedisplay();
}

void graphicsThread(int argc, char **argv) {
  printf ("Hello from graphics thread\n");
  
  // register callbacks
  glutDisplayFunc(render);
  glutIdleFunc(render);

  // external entries callbacks
  glutKeyboardFunc (processNormalKeys);
  glutSpecialFunc(processSpecialKeys);

  // enter GLUT event processing cycle
  glutMainLoop();
  
}

void hardwareThread() {
  printf ("Hello from hardware thread\n");

  struct sigaction act;
  act.sa_handler = graphicsInterrupt;
  sigaction (SIGALRM, & act, 0);

  // --------------- arcade specific --------------------- //
  // add timer for 1/60 seconds to process graphics, 16.67 milliseconds
  ualarm(16670, 16670); // 16.67 ms
}



void processorThread() {
  printf("hello from processor\n");
  fflush(stdout);

  long int cyclesExecuted = 0;

  while (CPU_on) {
    // lock thread if the amount of time to execute the cycles exceeds 60hz
    // this will essentially perform a batch delay, instead of delaying on
    // each instruction. This synchronizes the instructions with the 60hz
    // timer, in batches. This means that approximately the same amount of
    // instructions that would be executed during the 60hz window on the 8080
    // are emulated similarly here.
    if ((cyclesExecuted * clock_time_milliseconds) > 16) {
      #ifdef DEBUG
      printf ("Locking execution %ld %f\n", cyclesExecuted, cyclesExecuted * clock_time_milliseconds);
      #endif
      
      std::unique_lock<std::mutex> lk(m);
      cv.wait_for(lk, std::chrono::microseconds(16670), []{return hardware_interrupt;});
    }
    // this will cascade into interrupt code
    
    // check if interrupts enabled, and an interrupt happened
    if (State8080_ie(state) && hardware_interrupt) { 
      hardware_interrupt = false;
      cyclesExecuted = 0; // reset cycles
           
      // get interrupt set by hardware  11AAA111
      unsigned char op = (State8080_popInterrupt(state) << 3) | 0xC7;
      Emulate8080Op(state, &op);
      State8080_setIE(state, 1);
    }
    else {
      int cycles = op_clockCycles(state); // how many cycles this should take
      cyclesExecuted += cycles;
      
      Emulate8080State(state);     
    }
  } 
}

void INITIALIZE_GRAPHICS(int argc, char **argv) {
  // init GLUT and create window
  glutInit(&argc, argv);

  glutInitWindowPosition(100, 100);
  glutInitWindowSize(W * 2, H * 2);
  glutInitDisplayMode(GLUT_RGBA | GLUT_SINGLE); // double?
  glutCreateWindow("Arcade Machine 8080");
}

int main (int argc, char **argv) {
  INITIALIZE_PROCESSOR(state, argv);
  INITIALIZE_GRAPHICS(argc, argv);

  CPU_on = true;
  
  std::thread graphics (graphicsThread, argc, argv); // graphics thread
  std::thread cpu (processorThread); // processor thread
  std::thread hardware (hardwareThread); // hardware thread

  // synchronize threads
  cpu.join();
  graphics.join();  
  hardware.join();
  
  return 0;
}

