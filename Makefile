all: tlegen satobs tleinfo

util:=src/TLE.o src/SGP4.o src/opt_util.o src/tle_loader.o

CFLAGS=-Wall

tlegen: src/tlegen.o $(util)
	gcc -o tlegen ${CFLAGS} $^

satobs: src/satobs.o $(util)
	gcc -o satobs ${CFLAGS} $^

tleinfo: src/tleinfo.o $(util)
	gcc -o tleinfo ${CFLAGS} $^

src/%.o: src/%.c
	gcc -Wall -c $< -o $@

clean:
	rm -f tleinfo tlegen satobs src/*.o
