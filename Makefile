CC=clang
CFLAGS=

all: bmp

build:
	mkdir -p build

./build/bmp.o: ./src/bmp.c ./src/bmp.h build
	$(CC) $(CFLAGS) -c ./src/bmp.c
	mv bmp.o ./build

bmp: ./build/bmp.o ./src/main.c
	$(CC) $^ -o $@
	mv ./bmp ./bin/bmp

clear:
	rm ./build/* ./bin/* *.bmp