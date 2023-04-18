# Roux Institute | Spring 2023 | CS5700 Computer Networking
# HW4 Communicating States Across the Network
# file: Makefile
# Apr 15, 2023 | by Chun Sheung Ng

HDR = tic-tac-toe.h
SRC = tic-tac-toe.c

OBJ = ttt.o
GRD = makefile ${SRC} ${HDR}

CC = gcc
CFLAGS = -g0

all: ttt

ttt: tic-tac-toe.c tic-tac-toe.h
	$(CC) $(CFLAGS) tic-tac-toe.c -o ttt
	
#
# Clean up script
#
clean:
	/bin/rm -f *.o ttt