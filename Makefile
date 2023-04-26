CC=clang
CFLAGS=
BIN=./bin/bmp

all: bmp

build:
	mkdir -p build

./build/main.o: ./src/main.c build
	$(CC) $(CFLAGS) -c ./src/main.c -o ./build/main.o

./build/bmp.o: ./src/bmp.c ./src/bmp.h build
	$(CC) $(CFLAGS) -c ./src/bmp.c -o ./build/bmp.o

./build/transformations.o: ./src/transformations.c ./src/transformations.h build
	$(CC) $(CFLAGS) -c ./src/transformations.c
	mv transformations.o ./build

bmp: ./build/bmp.o ./build/transformations.o ./build/main.o
	$(CC) $(CFLAGS) -lm $^ -o $@
	mv ./bmp ./bin/bmp

clean:
	rm ./build/* ./bin/* *.bmp