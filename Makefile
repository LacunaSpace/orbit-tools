# @configure_input@

# Package-specific substitution variables
package = @PACKAGE_NAME@
version = @PACKAGE_VERSION@
tarname = @PACKAGE_TARNAME@

# Prefix-specific substituion variables
prefix = @prefix@
exec_prefix = @exec_prefix@
bindir = @bindir@

all: bin/tlegen bin/sattrack bin/satpass bin/tleinfo bin/termgen 

util:=build/TLE.o build/SGP4.o build/opt_util.o build/tle_loader.o build/observer.o build/util.o build/output.o

CFLAGS=-Wall -Isrc
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

install:
	install -d $(DESTDIR)$(bindir)
	install -m 0755 bin/tlegen $(DESTDIR)$(bindir)
	install -m 0755 bin/sattrack $(DESTDIR)$(bindir)
	install -m 0755 bin/satpass $(DESTDIR)$(bindir)
	install -m 0755 bin/tleinfo $(DESTDIR)$(bindir)
	install -m 0755 bin/termgen $(DESTDIR)$(bindir)

    

$(shell mkdir -p build bin)
