CFLAGS	= -g -Wall -pedantic -std=c99

LIB	= -lfl
SRC	= ice9.l ice9.y

ice9:	ice9.yy.o ice9.tab.o ast.o semantic.o main.o
	g++ -o $@ $^ $(LIB)

ice9.tab.c: ice9.y
	bison -d -t -v -o $@ $<

ice9.tab.h: ice9.y
	bison -d -o ice9.tab.c $<

ice9.tab.o: ice9.tab.c ast.h
	gcc $(CFLAGS) -c $<

ice9.yy.c: ice9.l ice9.tab.h
	flex -o$@ $<

ice9.yy.o: ice9.yy.c ast.h
	gcc $(CFLAGS) -c $<

ast.o: ast.c ast.h
	gcc $(CFLAGS) -c $<

semantic.o: semantic.cpp semantic.h parse.h
	g++ -g -c $<

main.o: main.cpp semantic.h
	g++ -g -c $<

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
	wc -lwc ice9.y ice9.l ast.h ast.c *.cpp semantic.h parse.h
