#--------------
# Makefile for 8080 Processor
# Author : Antony Toron
#--------------

# Macros
CC = gcc

CFLAGS =
# CFLAGS = -g
# CFLAGS = -D NDEBUG
# CFLAGS = -D NDEBUG -O

# Pattern rule, any .o file with .c file of same name will assume it
# %.o: %.c
#       $ (CC) $ (CFLAGS) -c $<

# Dependency rules for non-file targets
all: disassemble cpu

# clean directory
clobber: clean
	rm -f *~ \#*\#
clean:
	rm -f disassemble *.o

# Dependency rules for file targets
# include all files ncessary for building in -o shortcut case
cpu: CPU.o
	$(CC) $(CFLAGS) $< -o $@
disassemble: disassemble.o disassembler.o
	$(CC) $(CFLAGS) $< disassembler.o -o $@

# object file dependencies in recipes for all binary files
CPU.o: CPU.c
	$(CC) $(CFLAGS) -c $<
disassemble.o: disassemble.c disassembler.h
	$(CC) $(CFLAGS) -c $<
disassembler.o: disassembler.c disassembler.h
	$(CC) $(CFLAGS) -c $<
