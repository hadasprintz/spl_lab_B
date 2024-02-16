ll: myshell mypipeline looper
myshell: myshell.o LineParser.o
	gcc -m32 -Wall -g -o myshell myshell.o LineParser.o
mypipeline: mypipeline.o
	gcc -m32 -Wall -g -o mypipeline mypipeline.o
looper: looper.o
	gcc -m32 -Wall -g -o looper looper.o
LineParser.o: LineParser.h
	gcc -m32 -Wall -g -c LineParser.c -o LineParser.o
myshell.o: LineParser.h
	gcc -m32 -Wall -g -c myshell.c -o myshell.o
mypipeline.o:
	gcc -m32 -Wall -g -c mypipeline.c -o mypipeline.o
looper.o:
	gcc -m32 -Wall -g -c looper.c -o looper.o
.PHONEY : clean
clean:
	rm -f *.o all
