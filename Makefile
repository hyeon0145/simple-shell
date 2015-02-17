all:
	gcc -o smsh -ansi -std=c99 -D_BSD_SOURCE -D_XOPEN_SOURCE=500 *.c
