all: task0
task0: main.o start.o
	ld -m elf_i386 main.o util.o start.o -o task0
main.o: main.c util.c util.h
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector util.c -o util.o
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector main.c -o main.o
start.o: start.s
	nasm -f elf32 start.s -o start.o
.PHONY: clean
clean:
	rm -f *.o

