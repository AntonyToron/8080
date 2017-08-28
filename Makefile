#--------------
# Makefile for 8080 Processor
# Author : Antony Toron
#--------------

# Macros
CC = gcc
CPP = g++
CFLAGS = -g
#CFLAGS = -g -D CPU_DIAGNOSTIC
#CFLAGS = -D INSTRUCTION_DEBUGGING
#CFLAGS = -pg
#CFLAGS = -D DEBUG
# CFLAGS = -g
# CFLAGS = -D NDEBUG
# CFLAGS = -D NDEBUG -O
CPPFLAGS = -std=c++11 -g
# -lGLEW -lglut-lGLEW -lglut
#CPPLIBS = -lglfw3 -lm -lGL -lGLEW -lglut -lGLU -lpthread -lX11 -lXxf86vm -lXrandr -lXi -ldl -lXinerama -lXcursor -Wl,-Bstatic -lSDL2 -lSDL2_mixer -Wl,--as-needed
CPPLIBS = -lglfw3 -lm -lGL -lGLEW -lglut -lGLU -lpthread -lX11 -lXxf86vm -lXrandr -lXi -ldl -lXinerama -lXcursor -lSDL2 -lSDL2_mixer -static-libgcc
LIBS =  -lglfw3 -lm -lGL -lGLEW -lglut -lGLU -lpthread -lX11 -lXxf86vm -lXrandr -lXi -ldl -lXinerama -lXcursor -lSDL2 -lSDL2_mixer
WX_LIBS = $(shell wx-config --libs)
WX_FLAGS = $(shell wx-config --cxxflags)
OPTIONAL = #-static
#OPTIONAL = -static-libgcc # this will force all of the shared (.so) libraries to be
# not be used and the .a (static) to be used, so executable bundles everything
#LIBS += $(WX_LIBS)

# Pattern rule, any .o file with .c file of same name will assume it
# %.o: %.c
#       $ (CC) $ (CFLAGS) -c $<

# Dependency rules for non-file targets
all: disassemble c_arcade arcade playground test emulator

# clean directory
clobber: clean
	rm -f *~ \#*\#
clean:
	rm -f disassemble *.o

# Dependency rules for file targets
# include all files ncessary for building in -o shortcut case
c_arcade: 8080Arcade.o CPU.o Utils.o Drivers.o
	$(CC) $(CFLAGS) $< CPU.o Utils.o Drivers.o -o $@ $(LIBS)
disassemble: disassemble.o disassembler.o
	$(CC) $(CFLAGS) $< disassembler.o -o $@

test: cpu_test.o CPU.o Utils.o Drivers.o
	$(CC) $(CFLAGS) -g $< CPU.o Utils.o Drivers.o -o $@ $(LIBS)

playground: playground.o
	$(CPP) $(CFLAGS) $< -o $@ $(LIBS)
arcade: arcade_machine.o CPU.o Utils.o Drivers.o
	$(CPP) $(CFLAGS) $(OPTIONAL) $< CPU.o Utils.o Drivers.o -o $@ $(LIBS) #$(CPPLIBS)

emulator: emulator.o arcade_machine_library.o CPU.o Utils.o Drivers.o
	$(CPP) $(CFLAGS) $(WX_FLAGS) $< arcade_machine_library.o CPU.o Utils.o Drivers.o -o $@ $(WX_LIBS) $(LIBS)


# object file dependencies in recipes for all binary files
8080Arcade.o: 8080Arcade.c CPU.h Utils.h Drivers.h
	$(CC) $(CFLAGS) -c $<
CPU.o: CPU.c CPU.h
	$(CC) $(CFLAGS) -c $<
Utils.o: Utils.c Utils.h
	$(CC) $(CFLAGS) -c $<
Drivers.o: Drivers.c Drivers.h
	$(CC) $(CFLAGS) -c $<

cpu_test.o: cpu_test.c CPU.h Utils.h Drivers.h
	$(CC) $(CFLAGS) -c $<

playground.o: playground.cpp
	$(CPP) $(CFLAGS) -c $< $(LIBS)
arcade_machine.o: arcade_machine.cpp
	$(CPP) $(CPPFLAGS) -c $< $(LIBS)
arcade_machine_library.o: arcade_machine_library.cpp
	$(CPP) $(CPPFLAGS) -c $< $(LIBS)

emulator.o: emulator.cpp
	$(CPP) $(CPPFLAGS) $(WX_FLAGS) -c $< $(WX_LIBS) $(LIBS)


disassemble.o: disassemble.c disassembler.h
	$(CC) $(CFLAGS) -c $<
disassembler.o: disassembler.c disassembler.h
	$(CC) $(CFLAGS) -c $<
