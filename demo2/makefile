.RECIPEPREFIX = >

prog: main.o fooParser.tab.o lex.yy_foo_.o
> g++ $^ -o $@

main.o: main.cc FooLexer.hh fooParser.tab.hh location_t.hh
> g++ -c $< -o $@

lex.yy_foo_.o: lex.yy_foo_.cc FooLexer.hh fooParser.tab.hh location_t.hh
> g++ -c $< -o $@

fooParser.tab.o: fooParser.tab.cc FooLexer.hh location_t.hh
> g++ -c $< -o $@

lex.yy_foo_.cc: fooLexer.ll
> flex $<

fooParser.tab.hh fooParser.tab.cc fooParser.output: fooParser.yy
> bison $<

.PHONY: clean
clean:
> rm -f prog main.o lex.* fooParser.tab.* fooParser.output