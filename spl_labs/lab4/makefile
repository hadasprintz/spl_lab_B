all: hexeditplus
hexeditplus: hexeditplus.o
	gcc -m32 -Wall -g -o hexeditplus hexeditplus.o
hexeditplus.o:
	gcc -m32 -Wall -g -c hexeditplus.c -o hexeditplus.o
.PHONEY : clean
clean:
	rm -f *.o all



