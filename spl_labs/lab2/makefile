all: looper myShell
looper: looper.o
	gcc -m32 -Wall -g -o looper looper.o
myShell: myShell.o LineParser.o
	gcc -m32 -Wall -g -o myShell myShell.o LineParser.o
LineParser.o: LineParser.h
	gcc -m32 -Wall -g -c LineParser.c -o LineParser.o
looper.o:
	gcc -m32 -Wall -g -c looper.c -o looper.o
myShell.o: LineParser.h
	gcc -m32 -Wall -g -c myShell.c -o myShell.o
.PHONEY : clean
clean:
	rm -f *.o all