# nmake makefile
#
# Tools used:
#  Compile::Watcom Resource Compiler
#  Compile::GNU C
#  Make: nmake or GNU make
all : wintree.exe

wintree.exe : wintree.obj wintree.def
	gcc -Zomf wintree.obj wintree.def -o wintree.exe


wintree.obj : wintree.c 
	gcc -Wall -Zomf -c -O2 wintree.c -o wintree.obj

clean :
	rm -rf *exe *RES *obj