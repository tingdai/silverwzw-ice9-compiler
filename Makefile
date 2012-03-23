CFLAGS	= -g -Wall -pedantic -std=c99

LIB	= -lfl
SRC	= ice9.l ice9.y

ice9:	ice9.yy.o ice9.tab.o
	gcc -o $@ $^ $(LIB)

ice9.tab.c: ice9.y
	bison -d -t -v -o $@ $<

ice9.tab.h: ice9.y
	bison -d -o ice9.tab.c $<
	rm -f ice9.tab.c

ice9.tab.o: ice9.tab.c
	gcc $(CFLAGS) -c $<

ice9.yy.c: ice9.l ice9.tab.h
	flex -o$@ $<

ice9.yy.o: ice9.yy.c
	gcc $(CFLAGS) -c $<

clean:
	rm -f *.o ice9.output

cleanest:
	make clean
	rm -f ice9.tab.c ice9.yy.c ice9.tab.h ice9
remk:
	make clean
	rm -f ice9
	make
remkall:
	make cleanest
	make
