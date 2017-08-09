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
all: disassemble arcade

# clean directory
clobber: clean
	rm -f *~ \#*\#
clean:
	rm -f disassemble *.o

# Dependency rules for file targets
# include all files ncessary for building in -o shortcut case
arcade: 8080Arcade.o CPU.o Utils.o
	$(CC) $(CFLAGS) $< CPU.o Utils.o -o $@
disassemble: disassemble.o disassembler.o
	$(CC) $(CFLAGS) $< disassembler.o -o $@

# object file dependencies in recipes for all binary files
8080Arcade.o: 8080Arcade.c CPU.h Utils.h
	$(CC) $(CFLAGS) -c $<
CPU.o: CPU.c CPU.h
	$(CC) $(CFLAGS) -c $<
Utils.o: Utils.c Utils.h
	$(CC) $(CFLAGS) -c $<


disassemble.o: disassemble.c disassembler.h
	$(CC) $(CFLAGS) -c $<
disassembler.o: disassembler.c disassembler.h
	$(CC) $(CFLAGS) -c $<
