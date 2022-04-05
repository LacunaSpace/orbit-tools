#!/usr/local/bin/perl

print <<EOF;
#include "geo.h"

city cities[] = {
EOF
$count=0;
while(<>) {
!$count++ && next;

($city, $city_ascii, $lat, $lon, $country, $iso2, $iso3, $admin_name, $capital, $pop, $id) = $_ =~ /\"([^"]*)\"/g;
$lat =~ s/\"//g;
$lon =~ s/\"//g;
print <<EOF
{ "$city", "$city_ascii", $lon, $lat },
EOF
}

print <<EOF;
};

size_t nr_cities=$count;
EOF
