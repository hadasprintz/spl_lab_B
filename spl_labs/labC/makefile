ll: myShell mypipeline
myShell: myShell.o LineParser.o
	gcc -m32 -Wall -g -o myShell myShell.o LineParser.o
mypipeline: mypipeline.o
	gcc -m32 -Wall -g -o mypipeline mypipeline.o
LineParser.o: LineParser.h
	gcc -m32 -Wall -g -c LineParser.c -o LineParser.o
myShell.o: LineParser.h
	gcc -m32 -Wall -g -c myShell.c -o myShell.o
mypipeline.o:
	gcc -m32 -Wall -g -c mypipeline.c -o mypipeline.o
.PHONEY : clean
clean:
	rm -f *.o all
