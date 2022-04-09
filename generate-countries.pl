#!/usr/local/bin/perl

print <<EOF;
#include "geo.h"

country countries[] = {
EOF
$count=0;
while(<>) {
($country_code, $lat, $lon, $country) = $_ =~ /([^\s]+)\s+([^\s]+)\s+([^\s]+)\s+(.*)/;
print <<EOF;
{ "$country", "$country_code", $lon, $lat },
EOF
$count++;
}
close FH;

print <<EOF;
};

size_t nr_countries=$count;
EOF
