Overview
========
This project contains the following tools:
* `satpass` calculates satellite passes at a given location, given a TLE file
* `sattrack` calculates the location of satellite at a given point in time
  or period, and optionally its visibility from a location on earth
* `termgen` generates the coordinates of well-known places on earth, in a format
  that can be used by `satpass` and `sattrack`
* `tleinfo` reads one or more TLE-files and presents their contents in human-readable
  format
* `tlegen` generates TLE-files describing the orbits of simulated satellites.

Each tools contains built-in help that can be accessed by invoking it with the
`--help` option. Additional details can be found below.

Building
========
Building requires gnu make, gcc and perl5. Simply type `make`, executables will be placed
in the `bin` directory.

Details
=======
Date and time format
--------------------
All dates and times displayed by the tools are in UTC. Whenever, a date and time
is used as input, this is also assumed to be in UTC. The expected format is
`yyyy-mm-ddThh:mm:ssZ`. In some cases, the shorter format `yyyy-mm-dd` can also
be used, which is an abbreviation for `yyyy-mm-ddT00:00:00Z`.

`satpass`
---------
`satpass` calculates passes of a satellite for a given location on earth. The simplest
way to invoke `satpass` is as follows:

```
satpass --location=40,50 /path/to/TLE.txt
```
This will show the next time, calculated from the current date time, that the satellite
described by the TLE in `/path/to/TLE.txt` will be visible at the location with
latitude 40° and longitude 50°. 

The TLE file must have a format like this:

```
ls2b
1 47948U 21022S   22101.69270700  .00009630  00000-0  61914-3 0  9998
2 47948  97.5413   3.2896 0020316  16.1983 343.9891 15.08445861 57844
```

The first line, containing the satellite name, is optional. The file may contain 
one or more satellites. If it contains more than one satellite `satpass` will show
the first pass of any of the satellites, unless one specific satellite is selected
with the `--name=<NAME>` option.

Instead of a filename, the special value `-` may also be used to use `stdin` as input.
If no filename at all is specified, `satpass` will use the contents of environment
variable `$ORBIT_TOOLS_TLE` as the filename if it is set. This is useful in case 
there is a fixed set of satellite you are monitoring - set up a cronjob to periodically
download recent copies of their TLEs to a local file, and make `$ORBIT_TOOLS_TLE`
point at this file.

By default, `satpass` shows just one pass. This can be changed with the `--count=<COUNT>`
option. `satpass` can show its output in two ways: as rows, which is easier to read for
a human, and in columns, which is easier to parse for a computer program. The format
can be selected with `--format=<FORMAT>`, where `<FORMAT>` is either `rows` or `cols`.
By default, if only one result is requested, the `rows` format is used, and `cols` 
is used in case of multiple results.
The fields that are displayed per pass are satellite name, pass duration, start, TCA,
and end, and its highest elevation. This can be changed to include additional fields,
such as the azimuth at the highest elevation, or exclude some of the fields, with the
`--fields=<FIELDS>` string (use `satpass --help` for a list of possible fields).

By default, passes with an elevation of 0° or higher are shown. This can be changed
with the `--min-elevation` option.

Finally, by default passes that happen after the current date and time are displayed.
This can be changed with the `--start=<STARTDATE>` option.

The following example will show the first 3 passes of the year 2022 of the ls2b satellite
with an elevation of at least 30° in Amsterdam, and format the results as 'human readable'
rows:
```
satpass --location=52.3667,4.8833 --min-elevation=30 --name=ls2b \
    --start=2022-01-01 --count=3 --format=rows /path/to/TLE/tle
```

The following example will do the same, but using `termgen` instead of location as coordinates
(see `termgen` details), and only showing pass start and end, as UNIX timestamps, and formatted
as columns:
```
satpass --location=$(termgen Amsterdam) --min-elevation=30 --name=ls2b \
    --start=2022-01-01 --count=3 --format=cols --fields=SE /path/to/TLE/tle
```

The following example will show the next pass of any of the satellites in the TLE file
specified with the `$ORBIT_TOOLS_TLE` environment variable from now, with an elevation of
at least 50° (note the the `-e` option is a shorter alternative for `--min-elevation`):
```
satpass -e 50
```

For a comprehensive overview of options, use `satpass --help`.

`sattrack`
----------
`sattrack` shows the location of a satellite at a specified date and time. The
simplest invocation of `sattrack` would be:
```
sattrack --name=ls2b /path/to/TLE.txt
```
Like `satpass`, this will read TLES from the file `/path/to/TLE.txt` - however,
if this file contains more than one TLE, and no satellite name is specified with
the `--name` option, the first TLE from the file will be used. Again, `-` can be
used as filename to read from `stdin`, and if no filename at all is given, 
`$ORBIT_TOOLS_TLE` will be consulted.

By default, the satellite's location at the given date and time will be shown. This
can be changed by the `--start` option, which allows to specify an alternative starttime.

When not only the satellite's own location, but also its visibility from a certain
location is required, specify that location with the `--location` switch:

```
sattrack --name=ls2b --location=52.3667,4.8833 /path/to/TLE.txt
```

To make a real-time display of this, use (this uses `termgen` again instead
of specifying the location's coordinates directly):
```
while [ 1 ]; do
    clear
    sattrack --name=ls2b --location=$(termgen Amsterdam) /path/to/TLE.txt
    sleep 1
done
```
(or use `watch` if you have it installed)

It is also possible to show a satellite's location at multiple points in time.
To do this, use the `--count` and `--interval` options to generate `count` locations
with `interval` seconds intervals, starting at the time specified with `--start`.

Like `satpass`, both a human-readable row-oriented output, and a machine-readable
column-oriented output are available, and by default, the row-oriented output is
used when `count` is 1, the column-oriented otherwise. It can be explicitly set
with the `--format` option.

The output can further be modified with the `--fields` option. Consults the built-in
help for a complete list of fields.

The following example uses `sattrack` in combination with `gnuplot` to plot the elevation
of the ls2b satellite during its 10 minute pass of the location at 0°/0° at 
the 1st of June 2022 (note that the `--location=0,0` option could have omitted in
this case since it's the default):

```
sattrack --name=ls2b --location=0,0 --start=2022-06-01T21:59:04Z \
    --interval=10 --count=60 -fields=Tl |\
    gnuplot -p -e "plot '-' using 1:2"  
```


`termgen`
---------
`termgen` generates the coordinates of named locations on earth (its name, 'terminal
generator', refers to the ambition to be able to generate the locations of multiple
terminals, for example a large number of terminals randomly distributed across a 
country, for simulation purposes - it doesn't do that yet).

It can be be used with either city as argument:
```
termgen Amsterdam
```
or a two-letter country-code:
```
termgen NL
```
In the latter case, it will generate the coordinates of the country's geographic
center.

As shown before, the output can be directly substituted in the `--location` option
of `satpass` or `sattrack`:
```
satpass --location=$(termgen Capetown) /path/to/TLE.txt
```

`tleinfo`
---------
`tleinfo` reads one or more TLEs from a file, and shows the contents in a human-readable
format. If used like this:
```
tleinfo /path/to/TLE.txt
```
the data from all TLEs in the given file (which may also be `-` to read from `stdin`,
or omitted to use the contents of `$ORBIT_TOOLS_TLE`) will be shown. 

To show just the data from a specific satellite, use the `--name` option:
```
tleinfo --name=ls1 /path/to/TLE.txt
```

`tlegen`
--------
`tlegen` generates TLEs for simulation purposes.

When used without arguments, it will generate a TLE with all default values,
which is a sun-synchronous orbit with a 97.7° inclination and a mean motion (the
number of revolutions per day) of 15:

```
tlegen
```

If you are curious to find out the altitude of such an orbit, simply use the 
output directly with `sattrack`:
```
tlegen | sattrack --fields=A -
```

To set fields in the TLE to specific values, options such as `--inclination`,
`--b-star` and `--epoch` can be used. Use `tlegen --help` for a comprehensive
overview.

There are also options to set TLE value indirectly. For example, a TLE does
not contain an orbit's altitude directly, but an altitude of for example 1000km
can be specified as follows:
```
tlegen --altitude=1000
```
This will calculate and set the values for mean motion (and eccentricity in case
not one altitude, but a separate altitude at apogee and perigee, are specified)
accordingly.

Finally, there is an option to create a TLE that has the satellite arrive at a certain
location at a certain time. This is a new feature and should be considered experimental -
in particular, it does not go together well with some other options.
It can be extremely useful for certain tests, however.

The following example creates a TLE for a satellite that reaches Amsterdam on the
3rd of July at noon, then uses `satpass` to verify that it does indeed:

```
tlegen --epoch=2022-07-03 --target-time=2022-07-03T12:00:00Z \
    --target-location=$(termgen Amsterdam) |\
    satpass --location=$(termgen Amsterdam) --start=2022-07-03T11:00:00Z -
```



