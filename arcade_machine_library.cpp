/*
  File name: arcade_machine_library.cpp

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
#include <iomanip>

#include "arcade_machine_library.h"

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
//#include <GL/glut.h>
#include <GLFW/glfw3.h>

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

GLFWwindow *window;


std::mutex m;
std::mutex m2;
std::condition_variable cv;
std::unique_lock<std::mutex> lock(m);

struct itimerval timer;


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

void ArcadeOut3 (uint8_t ac) {
  Arcade8080_write3(ac, am_ports);
}

void ArcadeOut5 (uint8_t ac) {
  Arcade8080_write5(ac, am_ports);
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

  // OUT
  config_drivers_out_port(drivers, &ArcadeOut4, 4); // shift registers
  config_drivers_out_port(drivers, &ArcadeOut2, 2);
  config_drivers_out_port(drivers, &ArcadeOut3, 3); // sound
  config_drivers_out_port(drivers, &ArcadeOut5, 5);

  // IN
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

// ------------------------- BALLOON BOMBER STATE ------------------------

void LOAD_ROM_ballbomb (State8080_T state) {
  // load into memory
  size_t r1, r2, r3, r4, r5, r6, r7;
  unsigned char *tn01 = readFileIntoBuffer("./ROMS/tn01", &r1);
  unsigned char *tn02 = readFileIntoBuffer("./ROMS/tn02", &r2);
  unsigned char *tn03 = readFileIntoBuffer("./ROMS/tn03", &r3);
  unsigned char *tn04 = readFileIntoBuffer("./ROMS/tn04", &r4);
  unsigned char *tn05 = readFileIntoBuffer("./ROMS/tn05-1", &r5);
  unsigned char *tn06 = readFileIntoBuffer("./ROMS/tn06", &r6);
  unsigned char *tn07 = readFileIntoBuffer("./ROMS/tn07", &r7);

  // load the rom
  State8080_load_mem(state, 0x0000, r1, tn01);
  State8080_load_mem(state, 0x0800, r2, tn02);
  State8080_load_mem(state, 0x1000, r3, tn03);
  State8080_load_mem(state, 0x1800, r4, tn04);
  State8080_load_mem(state, 0x4000, r5, tn05);

  // color mapping
  //State8080_load_mem(state, 0x0000, r1, tn06);
  //State8080_load_mem(state, 0x0000, r1, tn07);
    
  printf ("Successfully loaded ROM\n");
  
}

State8080_T INIT_STATE_ballbomb () {
  state = State8080_init ();

  State8080_config_drivers_default(state, ArcadeDrivers());
  
  return state;
}

// -------------------------------------------------------------------

void INITIALIZE_PROCESSOR(State8080_T state, char **argv) {
  if (strcmp(argv[1], "invaders") == 0) {
    state = INIT_STATE_invaders ();

    LOAD_ROM_invaders (state);                          
  }
  else if (strcmp(argv[1], "ballbomb") == 0) {
    state = INIT_STATE_ballbomb ();

    LOAD_ROM_ballbomb (state);
  }
  else {
    fprintf (stderr, "This ROM is not supported\n");
    exit(1);
  }
}

void INITIALIZE_PROCESSOR_extern(State8080_T state, ROM rom) {
  switch (rom) {
  case INVADERS:
    state = INIT_STATE_invaders ();

    LOAD_ROM_invaders (state);  
    break;
  case BALLOON_BOMBER:
    state = INIT_STATE_ballbomb ();

    LOAD_ROM_ballbomb (state);
    
    break;
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
  // ADDS A LOCK ON RENDERING
  std::unique_lock<std::mutex> lk(m2);
  auto now = std::chrono::system_clock::now();
  cv.wait_until(lk, now + std::chrono::milliseconds(16), [](){return hardware_interrupt;});

  // POPULATE WINDOW WITH IN-MEMORY VIDEO RAM, (effective 60hz draw)
  populateWindowFromMemory();
  
  // RENDER SCREEN
  
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  glRasterPos2f(-1, 1); // IMPORTANT FOR STARTING AT 0, 0
  glPixelZoom(1 * 2, -1 * 2);   // MIRROR IMAGE, and ZOOM IN, to window size
  
  glDrawPixels(W, H, GL_RGBA, GL_UNSIGNED_BYTE, windowPixels);
  
  glfwSwapBuffers(window);
}

uint8_t port3_previous = 0x00;
uint8_t port5_previous = 0x00;

void playSoundEffects() {
  // check ports 3 and 5 for sound
  uint8_t port3_current = am_ports_get3(am_ports);
  uint8_t port5_current = am_ports_get5(am_ports);

  if (port3_current != port3_previous) {
    if ((port3_current & 0x02) && !(port3_previous & 0x02)) {
      playSoundEffect("shot");
    }
    if ((port3_current & 0x04) && !(port3_previous & 0x04)) {
      playSoundEffect("flash");
    }
    if ((port3_current & 0x08) && !(port3_previous & 0x08)) {
      playSoundEffect("invaderkilled");
    }

    // UFO, repeats
    if ((port3_current & 0x01) && !(port3_previous & 0x01)) {
      playSoundEffect("UFO_on");
    }
    if (!(port3_current & 0x01) && (port3_previous & 0x01)) {
      playSoundEffect("UFO_off");
    }
    
  }
  if (port5_current != port5_previous) {
    if ((port5_current & 0x01) && !(port5_previous & 0x02)) {
      playSoundEffect("mov1");
    }
    if ((port5_current & 0x02) && !(port5_previous & 0x02)) {
      playSoundEffect("mov2");
    }
    if ((port5_current & 0x04) && !(port5_previous & 0x04)) {
      playSoundEffect("mov3");
    }
    if ((port5_current & 0x08) && !(port5_previous & 0x08)) {
      playSoundEffect("mov4");
    }
    if ((port5_current & 0x10) && !(port5_previous & 0x10)) {
      playSoundEffect("UFO_hit");
    }
  }
  
  port3_previous = port3_current;
  port5_previous = port5_current; 
}

#ifdef INTERRUPT_TIMING
auto t1 = std::chrono::high_resolution_clock::now();
#endif

void graphicsInterrupt(int value) {
  #ifdef INTERRUPT_TIMING
  auto t2 = std::chrono::high_resolution_clock::now();
  std::cout << "Took : " << std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count() << " ms\n";
  t1 = t2;
  #endif
  
  // PUSH AN INTERRUPT
  if (!hardware_interrupt) {
    if (last_interrupt == 1) {
      State8080_pushInterrupt(state, 2); // graphics interrupt
      last_interrupt = 2;
    }
    else {
      State8080_pushInterrupt(state, 1);
      last_interrupt = 1;
    }
  }

  // NOTIFY PROCESSOR OF INTERRUPT
  hardware_interrupt = true;
  cv.notify_all(); // notify of hardware interrupt
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  // ------------------ GAME INIT -------------------- //
  if (key == GLFW_KEY_C && action == GLFW_PRESS) {
    INSERT_COIN (am_ports);
  }
  if (key == GLFW_KEY_C && action == GLFW_RELEASE) {
    REGISTER_COIN (am_ports);
  }
  
  if (key == GLFW_KEY_ENTER && action == GLFW_PRESS) {
    P1_START_DOWN (am_ports);
  }
  if (key == GLFW_KEY_ENTER && action == GLFW_RELEASE) {
    P1_START_UP (am_ports);
  }

  // ----------------- GAME CONTROLS ------------------ //
  if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
    P1_SHOT_DOWN (am_ports);
  }
  if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE) {
    P1_SHOT_UP (am_ports);
  }

  // ----------------- MOVE CONTROLS ------------------ //
  if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
    P1_LEFT_DOWN (am_ports);
  }
  if (key == GLFW_KEY_LEFT && action == GLFW_RELEASE) {
    P1_LEFT_UP (am_ports);
  }
  if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
    P1_RIGHT_DOWN (am_ports);
  }
  if (key == GLFW_KEY_RIGHT && action == GLFW_RELEASE) {
    P1_RIGHT_UP (am_ports);
  }
}

void turnOffTimer() {
  struct itimerval it_val = { 0 };

  setitimer(ITIMER_REAL, &it_val, &timer);
  printf ("Turned off timer\n");
  
}

void * graphicsThread(void *x) {
  printf ("Hello from graphics thread\n");

  if (!glfwInit())
    exit(1);

  // create windowed mode window and opengl context
  window = glfwCreateWindow(448, 512, "Arcade Machine 8080", NULL, NULL);
  if (!window) {
    glfwTerminate();
    exit(1);
  }

  // make the window's context current
  glfwMakeContextCurrent(window);

  glfwSetKeyCallback(window, key_callback);

  while (!glfwWindowShouldClose(window)) {  
    // render here
    render();

    // register sound effects
    playSoundEffects();

    glfwPollEvents();
  }

  /*
    SHUT DOWN ALL THREADS AND TIMERS

    TURN OFF GRAPHICS, SOUND, CLEAN MEMORY

   */

  glfwTerminate();

  // turn off timer
  turnOffTimer();

  CPU_on = false; // turn off CPU (IMPORTANT)

  // unlock CPU if locked
  hardware_interrupt = true;
  cv.notify_all();

  // clean up memory
  
  State8080_free(state);
  am_ports_free(am_ports);
  CLEAN_AUDIO();

  printf ("Turning off Graphics\n");
  fflush(stdout);
  pthread_exit(NULL);
}

void * hardwareThread(void *x) {
  printf ("Hello from hardware thread\n");

  // --------------- arcade specific --------------------- //
  // add timer for 1/60 seconds to process graphics, 16.67 milliseconds
  struct sigaction sa;
  //struct itimerval timer; GLOBAL

  memset (&sa, 0, sizeof (sa));
  sa.sa_handler = &graphicsInterrupt;
  sigaction (SIGALRM, &sa, NULL); // COULD BE SIGVTALARM and be virtual time

  timer.it_value.tv_sec = 0;
  timer.it_value.tv_usec = 16000;

  timer.it_interval.tv_sec = 0;
  /*
    The following interval should be 16670 ns, or 16.67 ms.

    Experimentation with timing suggested that the amount of instructions
    before an interrupt was not slowing down the system, but the actual
    interval between interrupts was. An interrupt being executed faster than
    at 1/60 s (approximately twice as fast), added an appropriate amount of
    speedup for the system to run at approximately the speed as original
    8080 CPU.
    
    This could likely be due to system time in processing on threads, or a
    factor of 2 missing elsewhere in calculations that could cause this halving
    to produce close to correct results.

   */
  timer.it_interval.tv_usec = 8000;

  setitimer (ITIMER_REAL, &timer, NULL);

  printf ("Exiting the hardware thread\n");
  fflush(stdout);
  pthread_exit(NULL);
}

void * processorThread(void *x) {
  printf("hello from processor\n");
  fflush(stdout);

  long int cyclesExecuted = 0;
  int last = 0;
  int currentInterrupt = 0;
  
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

      cv.wait(lock, [](){return hardware_interrupt;});

      // when unlocked, register the fact that we have recieved a hardware
      // interrupt
      hardware_interrupt = false; // reset
      cyclesExecuted     = 0;     // reset cyclesExecuted (realize interrupt)

      // check if interrupts are enabled on the CPU, in which case an interrupt
      // should actually be performed
      if (State8080_ie(state)) {
	// this is for when the hardware thread and processor thread go out of
	// sync. This will happen quite often but happens due to inaccuracies
	// in timing between threads and execution times
	currentInterrupt  = State8080_popInterrupt(state);
	if (currentInterrupt == last) {
	  if (currentInterrupt = 1)
	    currentInterrupt = 2;
	  else if (currentInterrupt = 2)
	    currentInterrupt = 1;
	}
	last = currentInterrupt;
	
	unsigned char op = (currentInterrupt << 3) | 0xC7;
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

  printf ("Turning off the CPU\n");
  fflush(stdout);
  pthread_exit(NULL);
}

void RUN_EMULATOR(ROM rom) {
  INITIALIZE_PROCESSOR_extern(state, rom);

  INIT_AUDIO (); // sound

  CPU_on = true;

  pthread_t cpu, graphics, hardware;
  int x = 0, y = 0, z = 0;

  pthread_create(&cpu, NULL, processorThread, &x);
  pthread_create(&graphics, NULL, graphicsThread, &y);
  pthread_create(&hardware, NULL, hardwareThread, &z);

  pthread_join(cpu, NULL);
  pthread_join(graphics, NULL);
  pthread_join(hardware, NULL);

  printf ("Finished running ROM\n");
}
