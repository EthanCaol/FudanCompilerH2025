
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
	./build/main.out < ../../test/2.5c/1.txt
	./build/main.out < ../../test/2.5c/2.txt
	./build/main.out < ../../test/2.5c/3.txt
	./build/main.out < ../../test/2.5c/4.txt
