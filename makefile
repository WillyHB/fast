all: main

main: main.o stack.o db_linked_list.o output.o
	gcc -o term main.o stack.o db_linked_list.o output.o -lX11 -lXft

output.o: output.c
	gcc -c output.c -I/usr/include/freetype2 -I/usr/include/X11/Xft 

db_linked_list.o: db_linked_list.c
	gcc -c db_linked_list.c

stack.o: stack.c
	gcc -c stack.c

main.o: main.c
	gcc -c main.c -I/usr/include/freetype2 -I/usr/include/X11/Xft  
