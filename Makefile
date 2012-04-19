CFLAGS	= -Wall -pedantic -std=c99
DEBUG =
FIX =

LIB	= -lfl
SRC	= ice9.l ice9.y

ice9:	ice9.yy$(FIX).o ice9.tab$(FIX).o ast$(FIX).o semantic$(FIX).o main$(FIX).o
	g++ -o $@ $^ $(LIB)

ice9.tab.c: ice9.y
	bison -d -t -v -o $@ $<

ice9.tab.h: ice9.y
	bison -d -o ice9.tab.c $<

ice9.tab$(FIX).o: ice9.tab.c ast.h
	gcc $(DEBUG) $(CFLAGS) -c $< -o $@

ice9.yy.c: ice9.l ice9.tab.h
	flex -o$@ $<

ice9.yy$(FIX).o: ice9.yy.c ast.h
	gcc $(DEBUG) $(CFLAGS) -c $< -o $@

ast$(FIX).o: ast.c ast.h
	gcc $(DEBUG) $(CFLAGS) -c $< -o $@

semantic$(FIX).o: semantic.cpp semantic.h parse.h
	g++ $(DEBUG) -c $< -o $@

main$(FIX).o: main.cpp semantic.h
	g++ $(DEBUG) -c $< -o $@

instruct$(FIX).o: instruct.cpp instruct.h
	g++ $(DEBUG) -c $< -o $@

memmgr$(FIX).o: memmgr.cpp memmgr.h
	g++ $(DEBUG) -c $< -o $@

conststr$(FIX).o: constStr.cpp constStr.h
	g++ $(DEBUG) -c $< -o $@

clean:
	rm -f *.o ice9.output

cleanest:
	make clean
	rm -f ice9.tab.c ice9.yy.c ice9.tab.h ice9
debug:
	make DEBUG=-g FIX=.g
wc:
	wc -lwc ice9.y ice9.l ast.c *.cpp *.h
