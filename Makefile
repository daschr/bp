CC=gcc

all:
	$(CC) -pedantic -Wall -O3 -o bp bp.c -lm
