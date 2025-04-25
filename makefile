all: main

main: main.o stack.o
	gcc -o term main.o stack.o -lX11 -lXft

stack.o: stack.c
	gcc -c stack.c

main.o: main.c
	gcc -c main.c -I/usr/include/freetype2 -I/usr/include/X11/Xft 
