all: tlegen satobs

util:=src/TLE.o src/SGP4.o src/opt_util.o src/tle_loader.o

CFLAGS=-Wall

tlegen: src/tlegen.o $(util)
	gcc -o tlegen ${CFLAGS} $^

satobs: src/satobs.o $(util)
	gcc -o satobs ${CFLAGS} $^

src/%.o: src/%.c
	gcc -Wall -c $< -o $@

clean:
	rm -f tlegen satobs src/*.o
