all: lab5

lab5: lab5.o start.o
	ld -o lab5 lab5.o startup.o start.o -L/usr/lib32 -lc -T linking_script -dynamic-linker /lib32/ld-linux.so.2

start.o: start.s
	nasm -g -f elf32 -o start.o start.s

lab5.o: lab5.c
	gcc -m32 -c lab5.c -o lab5.o

.PHONY: clean
clean:
	rm -f lab5.o start.o lab5
