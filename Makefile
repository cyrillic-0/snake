all: build

build: main.cpp
	g++ main.cpp -o snake -lSDL2
