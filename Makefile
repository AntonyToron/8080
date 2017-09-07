#--------------
# Makefile for 8080 Processor
# Author : Antony Toron
#--------------

# Macros
CC = gcc
CPP = g++
CFLAGS = -g -O3
#CFLAGS = -g -D CPU_DIAGNOSTIC
#CFLAGS = -D INSTRUCTION_DEBUGGING
#CFLAGS = -pg
#CFLAGS = -D DEBUG
# CFLAGS = -g
# CFLAGS = -D NDEBUG
# CFLAGS = -D NDEBUG -O
CPPFLAGS = -std=c++11 -g -O3
#SDLLIBS = /usr/lib/x86_64-linux-gnu/libSDL2_mixer.a /usr/lib/x86_64-linux-gnu/libSDL2.a
#SDLLIBS = -Wl, -Bstatic $(shell sdl-config --static-libs) -Wl, -Bdynamic
#SDLLIBS = -Wl, -Bstatic -L/usr/lib/x86_64-linux-gnu -lSDL2 -lSDL2_mixer -lpthread -lm -ldl -lasound -lm -ldl -lpthread -lpulse-simple -lpulse -lX11 -lXext -L/usr/lib/x86_64-linux-gnu -lcaca -lpthread -Wl, -Bdynamic
CPPLIBS = -lglfw3 -lm -lGL -lGLEW -lglut -lGLU -lpthread -lX11 -lXxf86vm -lXrandr -lXi -ldl -lXinerama -lXcursor -lSDL2 -lSDL2_mixer -static-libgcc
#LIBS =  -lglfw3 -lm -lGL -lGLEW -lglut -lGLU -lXxf86vm -lXrandr -lXi  -lXinerama -ldl -lXcursor $(SDLLIBS) 
LIBS = -lglfw3 -lm -lGL -lGLEW -lglut -lGLU -lpthread -lX11 -lXxf86vm -lXrandr -lXi -ldl -lXinerama -lXcursor -lSDL2 -lSDL2_mixer
WX_LIBS = $(shell wx-config --libs)
WX_FLAGS = $(shell wx-config --cxxflags)
#
#SDLLIBS = -Wl, -Bstatic -lSDL2 -lSDL2_mixer -Wl, -Bdynamic

#LIBS = $(SDLLIBS) + $(LIBS)
INCLUDES = -I./hardware -I./utils -I./machine -I./obj
OBJS = ./obj/CPU.o ./obj/Utils.o ./obj/Drivers.o
HEADERS = ./machine/CPU.h ./utils/Utils.h ./hardware/Drivers.h ./machine/arcade_machine.h ./machine/Types.h

# Pattern rule, any .o file with .c file of same name will assume it
# %.o: %.c
#       $ (CC) $ (CFLAGS) -c $<

# Dependency rules for non-file targets
all: disassemble c_arcade arcade playground test emulator

# clean directory
clobber: clean
	rm -f *~ \#*\# ./obj/*~ ./machine/*~ ./utils/*~ ./hardware/*~ ./obj/\#*\# ./machine/\#*\# ./utils/\#*\# ./hardware/\#*\#

clean:
	rm -f *.o ./obj/*.o ./machine/*.o ./utils/*.o ./hardware/*.o
#	rm -rf $(GARBAGE)
#	rm -f disassemble *.o

# Dependency rules for file targets
# include all files ncessary for building in -o shortcut case
c_arcade: ./obj/8080Arcade.o $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) $< $(OBJS) -o $@ $(LIBS)
disassemble: ./obj/disassemble.o ./obj/disassembler.o
	$(CC) $(CFLAGS) $(INCLUDES) $< ./obj/disassembler.o -o $@

test: ./obj/cpu_test.o $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -g $< $(OBJS) -o $@ $(LIBS)

playground: ./obj/playground.o
	$(CPP) $(CFLAGS) $(INCLUDES) $< -o $@ $(LIBS)
arcade: ./obj/arcade_machine_run.o ./obj/arcade_machine.o $(OBJS)
	$(CPP) $(CFLAGS) $(OPTIONAL) $(INCLUDES) $< ./obj/arcade_machine.o $(OBJS) -o $@ $(LIBS)

emulator: ./obj/emulator.o ./obj/arcade_machine.o $(OBJS)
	$(CPP) $(CFLAGS) $(WX_FLAGS) $(INCLUDES) $< ./obj/arcade_machine.o $(OBJS) -o $@ $(WX_LIBS) $(LIBS)


# object file dependencies in recipes for all binary files
./obj/8080Arcade.o: 8080Arcade.c $(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
./obj/CPU.o: ./machine/CPU.c $(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
./obj/Utils.o: ./utils/Utils.c $(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
./obj/Drivers.o: ./hardware/Drivers.c $(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

./obj/cpu_test.o: cpu_test.c $(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

./obj/playground.o: playground.cpp
	$(CPP) $(CFLAGS) $(INCLUDES) -c $< $(LIBS) -o $@
./obj/arcade_machine.o: ./machine/arcade_machine.cpp $(HEADERS)
	$(CPP) $(CPPFLAGS) $(INCLUDES) -c $< $(LIBS) -o $@
./obj/arcade_machine_run.o: arcade_machine_run.cpp $(HEADERS)
	$(CPP) $(CPPFLAGS) $(INCLUDES) -c $< $(LIBS) -o $@

./obj/emulator.o: emulator.cpp
	$(CPP) $(CPPFLAGS) $(WX_FLAGS) $(INCLUDES) -c $< $(WX_LIBS) $(LIBS) -o $@

./obj/disassemble.o: ./machine/disassemble.c ./machine/disassembler.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
./obj/disassembler.o: ./machine/disassembler.c ./machine/disassembler.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
