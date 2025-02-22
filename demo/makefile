
all: calc.l calc.y
	bison -vdty calc.y
	flex calc.l
	gcc -o calc y.tab.c lex.yy.c

clean:
	rm -f calc y.tab.c y.tab.h lex.yy.c y.output