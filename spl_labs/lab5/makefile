all: loader

loader: loader.o start.o
	ld -o loader loader.o startup.o start.o -L/usr/lib32 -lc -T linking_script -dynamic-linker /lib32/ld-linux.so.2

start.o: start.s
	nasm -g -f elf32 -o start.o start.s

loader.o: loader.c
	gcc -m32 -c loader.c -o loader.o

.PHONY: clean
clean:
	rm -f lab5.o start.o loader
