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

`sattrack`
----------

`termgen`
---------

`tleinfo`
---------

`tlegen`
--------


