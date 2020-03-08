CC=gcc

all:
	$(CC) -pedantic -Wall -O3 -lm -o bp bp.c
