all: myELF
myELF: myELF.o
	gcc -m32 -g -Wall -o myELF myELF.o
myELF.o:
	gcc -m32 -g -Wall -c myELF.c -o myELF.o
.PHONEY : clean
clean:
	rm -f myELF.o all