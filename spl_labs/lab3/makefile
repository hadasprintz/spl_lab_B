all: task0 task1
task0: main.o start.o
	ld -m elf_i386 main.o util.o start.o -o task0
task1: task1.o
	gcc -m32 -no-pie -o task1 task1.o
main.o: main.c util.c util.h
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector util.c -o util.o
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector main.c -o main.o
start.o: start.s
	nasm -f elf32 start.s -o start.o
task1.o: task1.s
	nasm -f elf32 task1.s -o task1.o
.PHONY: cleam
clean:
	rm -f *.o

