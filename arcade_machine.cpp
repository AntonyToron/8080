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
#include <pthread.h>
#include <sys/time.h>

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
std::mutex m2;
std::condition_variable cv;
std::unique_lock<std::mutex> lock(m);


uint8_t f = 0;
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

uint8_t ArcadeRead1 () {
  return Arcade8080_read1(am_ports);
}

uint8_t ArcadeRead2 () {
  return Arcade8080_read2(am_ports);
}

Drivers_T ArcadeDrivers() {
  Drivers_T drivers = Drivers_init();
  am_ports = am_ports_init();

  // shift registers
  config_drivers_out_port(drivers, &ArcadeOut4, 4);
  config_drivers_out_port(drivers, &ArcadeOut2, 2);
  config_drivers_in_port(drivers, &ArcadeRead3, 3);
  config_drivers_in_port(drivers, &ArcadeRead0, 0);
  config_drivers_in_port(drivers, &ArcadeRead1, 1);
  config_drivers_in_port(drivers, &ArcadeRead2, 2);
  
  return drivers;
}


// -------------------------- INVADERS STATE ----------------------------

void LOAD_ROM_invaders (State8080_T state) {
  // load into memory
  size_t bytesRead;
  unsigned char *buffer = readFileIntoBuffer("invaders", &bytesRead);

  // load the rom
  State8080_load_mem(state, 0, bytesRead, buffer);
    
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
  std::unique_lock<std::mutex> lk(m2);
  auto now = std::chrono::system_clock::now();
  cv.wait_until(lk, now + std::chrono::milliseconds(16), [](){return hardware_interrupt;});

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
  else if (key == 'y') {
    startPrintingOut();
    f = 1;
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
  // PUSH AN INTERRUPT
  if (last_interrupt == 1) {
    State8080_pushInterrupt(state, 2); // graphics interrupt
    last_interrupt = 2;
  }
  else {
    State8080_pushInterrupt(state, 1);
    last_interrupt = 1;
  }

  // NOTIFY PROCESSOR OF INTERRUPT
  hardware_interrupt = true;
  cv.notify_all(); // notify of hardware interrupt

  // POPULATE WINDOW WITH IN-MEMORY VIDEO RAM, (effective 60hz draw)
  populateWindowFromMemory();
}

void graphicsInterruptCallback(int x) {
  graphicsInterrupt(0);
  
  glutPostRedisplay();
}

void * graphicsThread(void *x) {
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

void * hardwareThread(void *x) {
  printf ("Hello from hardware thread\n");

  // --------------- arcade specific --------------------- //
  // add timer for 1/60 seconds to process graphics, 16.67 milliseconds
  struct sigaction sa;
  struct itimerval timer;

  memset (&sa, 0, sizeof (sa));
  sa.sa_handler = &graphicsInterrupt;
  sigaction (SIGALRM, &sa, NULL); // COULD BE SIGVTALARM and be virtaul time

  timer.it_value.tv_sec = 0;
  timer.it_value.tv_usec = 16000;

  timer.it_interval.tv_sec = 0;
  timer.it_interval.tv_usec = 16000;

  setitimer (ITIMER_REAL, &timer, NULL);
}



void * processorThread(void *x) {
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
    if ((cyclesExecuted * clock_time_milliseconds) > 16.67) {
      // when too many instructions happen before interrupt,
      // pause execution of instructions until a hardware interrupt should
      // happen
      auto now = std::chrono::system_clock::now();
      cv.wait_until(lock, now + std::chrono::milliseconds(9), [](){return hardware_interrupt;}); // have to add some padding for inaccuracy in system time measurement (not waiting for exactly 16 milliseconds)

      // when unlocked, register the fact that we have recieved a hardware
      // interrupt
      hardware_interrupt = false; // reset
      cyclesExecuted     = 0;     // reset cyclesExecuted (realize interrupt)

      // check if interrupts are enabled on the CPU, in which case an interrupt
      // should actually be performed
      if (State8080_ie(state)) {
	unsigned char op = (State8080_popInterrupt(state) << 3) | 0xC7;
	cyclesExecuted += 11;      // cycles for RST
	Emulate8080Op(state, &op);
	State8080_setIE(state, 0); // IE is set to 0 before interrupt exec
      }
    }  
    else {  // ELSE, can continue executing instructions as usual
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
  /*
  std::thread graphics (graphicsThread, argc, argv); // graphics thread
  std::thread cpu (processorThread); // processor thread
  std::thread hardware (hardwareThread); // hardware thread

  // synchronize threads
  cpu.join();
  graphics.join();  
  hardware.join();
  */

  pthread_t cpu, graphics, hardware;
  int x = 0, y = 0, z = 0;

  pthread_create(&cpu, NULL, processorThread, &x);
  pthread_create(&graphics, NULL, graphicsThread, &y);
  pthread_create(&hardware, NULL, hardwareThread, &z);

  /*
  pthread_attr_t thAttr;
  int policy = 0;
  int min_prio_for_policy = 0;

  pthread_attr_init(&thAttr);
  pthread_attr_getschedpolicy(&thAttr, &policy);
  min_prio_for_policy = sched_get_priority_min(policy);

  pthread_setschedprio(graphics, min_prio_for_policy);
  pthread_attr_destroy(&thAttr);
  */
  /*
  struct sched_param p;
  p.sched_priority = 0;
  sched_setscheduler(graphics, SCHED_IDLE, &p);
  */

  pthread_join(cpu, NULL);
  pthread_join(graphics, NULL);
  pthread_join(hardware, NULL);
  
  return 0;
}

