encoder: encode.o
	gcc -m32 -Wall -g -o encode encode.o
encode.o:
	gcc -m32 -Wall -g -c encode.c -o encode.o
.PHONEY : clean
clean:
	rm -f *.o encoder
