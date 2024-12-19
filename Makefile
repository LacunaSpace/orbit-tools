all: bin/tlegen bin/sattrack bin/satpass bin/tleinfo bin/termgen bin/orbitcalc

util:=build/TLE.o build/SGP4.o build/opt_util.o build/tle_loader.o build/observer.o build/util.o build/output.o build/debug.o

version:=$(shell git describe --tags --always)

CFLAGS=-Wall -Isrc -DVERSION=\"$(version)\"
LDFLAGS=-lm

bin/tlegen: build/tlegen.o $(util)
	$(CC) -o bin/tlegen $^ ${LDFLAGS}

bin/sattrack: build/sattrack.o $(util)
	$(CC) -o bin/sattrack $^ ${LDFLAGS}

bin/satpass: build/satpass.o $(util)
	$(CC) -o bin/satpass $^ ${LDFLAGS}

bin/tleinfo: build/tleinfo.o $(util)
	$(CC) -o bin/tleinfo $^ ${LDFLAGS}

bin/termgen: build/termgen.o build/countries.o build/cities.o $(util)
	$(CC) -o bin/termgen $^ ${LDFLAGS}

bin/orbitcalc: build/orbitcalc.o build/orbitcalc_commands.o $(util)
	$(CC) -o bin/orbitcalc $^ ${LDFLAGS}

build/cities.o: build/cities.c
	$(CC) ${CFLAGS} -c $< -o $@

build/countries.o: build/countries.c
	$(CC) ${CFLAGS} -c $< -o $@

build/%.o: src/%.c
	$(CC) ${CFLAGS} -c $< -o $@

build/cities.c: src/worldcities.csv src/othercities.csv
	perl generate-cities.pl $^ > $@

build/countries.c: src/countries.txt
	perl generate-countries.pl $^ > $@

clean:
	rm -rf build bin

    

$(shell mkdir -p build bin)
