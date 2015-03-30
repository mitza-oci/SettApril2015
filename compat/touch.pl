#!/usr/bin/perl
use strict;
if (!-r $ARGV[0]) {
	open FH, '>' . $ARGV[0] or die "Couldn't write $ARGV[0]: $!";
	close FH;
}
else {
	my $mtime = time;
	utime($mtime, $mtime, $ARGV[0]) or die "Couldn't update $ARGV[0]: $!";
}
