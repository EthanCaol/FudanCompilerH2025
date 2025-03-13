
all:
	make -s clean
	make -s build
	make -s test
	make -s clean

build: main.cc
	mkdir -p build
	g++ -o build/main.out main.cc

clean:
	rm -rf build

test:
	./build/main.out < ../../test/3.12a/1.txt
	./build/main.out < ../../test/3.12a/2.txt