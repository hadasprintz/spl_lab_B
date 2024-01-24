all: AntiVirus

AntiVirus: AntiVirus.o
	gcc -m32 -Wall -g -o AntiVirus AntiVirus.o

AntiVirus.o:
	gcc -m32 -Wall -g -c AntiVirus.c -o AntiVirus.o

.PHONY: clean
clean:
	rm -f *.o all
