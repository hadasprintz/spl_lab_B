all: addresses base menu
addresses: addresses.o
	gcc -m32 -Wall -g -o addresses addresses.o
base: base.o
	gcc -m32 -Wall -g -o base base.o
menu: menu.o
	gcc -m32 -Wall -g -o menu menu.o
count-words.o:
	gcc -m32 -Wall -g -c count-words.c -o count-words.o
addresses.o:
	gcc -m32 -Wall -g -c addresses.c -o addresses.o
base.o:
	gcc -m32 -Wall -g -c base.c -o base.o
menu.o:
	gcc -m32 -Wall -g -c menu.c -o menu.o
.PHONEY : clean
clean:
	rm -f *.o all
