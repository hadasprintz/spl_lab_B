all: task0
task0: start.o main.o util.o
	ld -m elf_i386 start.o main.o util.o -o task0
start.o: start.s
	nasm -f elf32 -g start.s -o start.o
main.o: main.c util.h
	gcc -m32 -Wall -std=c99 -nostdlib -fno-stack-protector main.c -o main.o
util.o: util.c util.h
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector util.c -o util.o
clean:
	rm -f *.o all

