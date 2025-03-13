
all:
	make -s clean
	make -s build
	make -s test
	make -s clean

build: main.l main.y
	mkdir -p build
	bison -o build/y.tab.c -vdty -Wcounterexamples main.y 
	flex  -o build/lex.yy.c main.l
	gcc   -o build/main.out build/y.tab.c build/lex.yy.c

clean:
	rm -rf build

test:
	./build/main.out < ../../test/3.3d/1.txt
	./build/main.out < ../../test/3.3d/2.txt
	./build/main.out < ../../test/3.3d/3.txt