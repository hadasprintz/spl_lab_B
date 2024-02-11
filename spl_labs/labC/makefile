ll: myShell mypipeline looper
myShell: myShell.o LineParser.o
	gcc -m32 -Wall -g -o myShell myShell.o LineParser.o
mypipeline: mypipeline.o
	gcc -m32 -Wall -g -o mypipeline mypipeline.o
looper: looper.o
	gcc -m32 -Wall -g -o looper looper.o
LineParser.o: LineParser.h
	gcc -m32 -Wall -g -c LineParser.c -o LineParser.o
myShell.o: LineParser.h
	gcc -m32 -Wall -g -c myShell.c -o myShell.o
mypipeline.o:
	gcc -m32 -Wall -g -c mypipeline.c -o mypipeline.o
looper.o:
	gcc -m32 -Wall -g -c looper.c -o looper.o
.PHONEY : clean
clean:
	rm -f *.o all
