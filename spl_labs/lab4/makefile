all: hexeditplus task4
hexeditplus: hexeditplus.o
	gcc -m32 -g -Wall -o hexeditplus hexeditplus.o
task4: task4.o
	gcc -m32 -fno-pie -fno-stack-protector -g -Wall -o task4 task4.o
hexeditplus.o:
	gcc -m32 -g -Wall -c hexeditplus.c -o hexeditplus.o
task4.o:
	gcc -m32 -fno-pie -fno-stack-protector -g -Wall -c -o task4.o task4.c
.PHONEY : clean
clean:
	rm -f *.o all



