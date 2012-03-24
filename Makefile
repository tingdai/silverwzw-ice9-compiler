CFLAGS	= -g -Wall -pedantic

LIB	= -lfl
SRC	= ice9.l ice9.y

ice9:	ice9.yy.o ice9.tab.o ast.o
	g++ -o $@ $^ $(LIB)

ice9.tab.c: ice9.y
	bison -d -t -v -o $@ $<

ice9.tab.h: ice9.y
	bison -d -o ice9.tab.c $<

ice9.tab.o: ice9.tab.c ast.h
	g++ $(CFLAGS) -c $<

ice9.yy.c: ice9.l ice9.tab.h
	flex -o$@ $<

ice9.yy.o: ice9.yy.c ast.h
	g++ $(CFLAGS) -c $<

ast.o: ast.c ast.h
	g++ $(CFLAGS) -c $<

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
wc:
	wc -lwc ice9.y ice9.l ast.h ast.c
