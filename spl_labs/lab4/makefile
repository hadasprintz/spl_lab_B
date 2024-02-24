all: hexeditplus digit_cnt
hexeditplus: hexeditplus.o
	gcc -m32 -Wall -g -o hexeditplus hexeditplus.o
digit_cnt: digit_cnt.o
	gcc -m32 -Wall -g -o digit_cnt digit_cnt.o
hexeditplus.o:
	gcc -m32 -Wall -g -c hexeditplus.c -o hexeditplus.o
digit_cnt.o:
	gcc -m32 -Wall -g -c digit_cnt.c -o digit_cnt.o
.PHONEY : clean
clean:
	rm -f *.o all



