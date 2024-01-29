all: looper myShell mypipe
looper: looper.o
	gcc -m32 -Wall -g -o looper looper.o
myShell: myShell.o LineParser.o
	gcc -m32 -Wall -g -o myShell myShell.o LineParser.o
mypipe: mypipe.o
	gcc -m32 -Wall -g -o mypipe mypipe.o
LineParser.o: LineParser.h
	gcc -m32 -Wall -g -c LineParser.c -o LineParser.o
looper.o:
	gcc -m32 -Wall -g -c looper.c -o looper.o
myShell.o: LineParser.h
	gcc -m32 -Wall -g -c myShell.c -o myShell.o
mypipe.o:
	gcc -m32 -Wall -g -c mypipe.c -o mypipe.o
.PHONEY : clean
clean:
	rm -f *.o all