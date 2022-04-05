all: bin/tlegen bin/satobs bin/tleinfo bin/termgen

util:=build/TLE.o build/SGP4.o build/opt_util.o build/tle_loader.o

CFLAGS=-Wall -Isrc

bin/tlegen: build/tlegen.o $(util)
	gcc -o bin/tlegen ${CFLAGS} $^

bin/satobs: build/satobs.o $(util)
	gcc -o bin/satobs ${CFLAGS} $^

bin/tleinfo: build/tleinfo.o $(util)
	gcc -o bin/tleinfo ${CFLAGS} $^

bin/termgen: build/termgen.o build/geo.o $(util)
	gcc -o bin/termgen ${CFLAGS} $^

build/geo.o: build/geo.c
	gcc ${CFLAGS} -c $< -o $@

build/%.o: src/%.c
	gcc ${CFLAGS} -c $< -o $@

build/geo.c: src/worldcities.csv src/othercities.csv
	./generate-geo.pl $^ > $@

clean:
	rm -rf build bin

$(shell mkdir -p build bin)
