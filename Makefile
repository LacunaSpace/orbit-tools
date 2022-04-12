all: bin/tlegen bin/sattrack bin/satpass bin/tleinfo bin/termgen 

util:=build/TLE.o build/SGP4.o build/opt_util.o build/tle_loader.o build/observer.o build/util.o build/output.o

CFLAGS=-Wall -Isrc

bin/tlegen: build/tlegen.o $(util)
	gcc -o bin/tlegen ${CFLAGS} $^

bin/sattrack: build/sattrack.o $(util)
	gcc -o bin/sattrack ${CFLAGS} $^

bin/satpass: build/satpass.o $(util)
	gcc -o bin/satpass ${CFLAGS} $^

bin/tleinfo: build/tleinfo.o $(util)
	gcc -o bin/tleinfo ${CFLAGS} $^

bin/termgen: build/termgen.o build/countries.o build/cities.o $(util)
	gcc -o bin/termgen ${CFLAGS} $^

build/cities.o: build/cities.c
	gcc ${CFLAGS} -c $< -o $@

build/countries.o: build/countries.c
	gcc ${CFLAGS} -c $< -o $@

build/%.o: src/%.c
	gcc ${CFLAGS} -c $< -o $@

build/cities.c: src/worldcities.csv src/othercities.csv
	./generate-cities.pl $^ > $@

build/countries.c: src/countries.txt
	./generate-countries.pl $^ > $@

clean:
	rm -rf build bin

$(shell mkdir -p build bin)
