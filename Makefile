all: make4061
	echo make4061

make4061: main.o util.o
	gcc -o make4061 main.o util.o

main.o: main.c
	gcc -c main.c

util.o: util.c
	gcc -c util.c

clean:
	rm -rf main.o util.o make4061
