all: bin/tlegen bin/sattrack bin/satpass bin/tleinfo bin/termgen 

util:=build/TLE.o build/SGP4.o build/opt_util.o build/tle_loader.o build/observer.o build/util.o build/output.o

CFLAGS=-Wall -Isrc
LDFLAGS=-lm

bin/tlegen: build/tlegen.o $(util)
	gcc -o bin/tlegen $^ ${LDFLAGS}

bin/sattrack: build/sattrack.o $(util)
	gcc -o bin/sattrack $^ ${LDFLAGS}

bin/satpass: build/satpass.o $(util)
	gcc -o bin/satpass $^ ${LDFLAGS}

bin/tleinfo: build/tleinfo.o $(util)
	gcc -o bin/tleinfo $^ ${LDFLAGS}

bin/termgen: build/termgen.o build/countries.o build/cities.o $(util)
	gcc -o bin/termgen $^ ${LDFLAGS}

build/cities.o: build/cities.c
	gcc ${CFLAGS} -c $< -o $@

build/countries.o: build/countries.c
	gcc ${CFLAGS} -c $< -o $@

build/%.o: src/%.c
	gcc ${CFLAGS} -c $< -o $@

build/cities.c: src/worldcities.csv src/othercities.csv
	perl generate-cities.pl $^ > $@

build/countries.c: src/countries.txt
	perl generate-countries.pl $^ > $@

clean:
	rm -rf build bin

$(shell mkdir -p build bin)
