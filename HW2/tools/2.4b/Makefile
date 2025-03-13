
all:
	make -s clean
	make -s build
	make -s test
	make -s clean

build: main.l
	mkdir -p build
	lex -o build/lex.yy.c main.l
	gcc build/lex.yy.c -o build/main.out -ll

clean:
	rm -rf build

test:
	./build/main.out < ../../test/2.4b/1.txt
	./build/main.out < ../../test/2.4b/2.txt
	./build/main.out < ../../test/2.4b/3.txt
