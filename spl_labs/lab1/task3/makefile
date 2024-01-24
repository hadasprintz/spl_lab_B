all: menu_map
menu_map: menu_map.o
	gcc -m32 -Wall -g -o menu_map menu_map.o
menu_map.o:
	gcc -m32 -Wall -g -c menu_map.c -o menu_map.o
.PHONEY : clean
clean:
	rm -f *.o all
