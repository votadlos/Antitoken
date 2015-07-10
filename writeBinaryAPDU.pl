#!/usr/bin/perl
# 
# Takes Smartcard Sniffer's (https://code.google.com/p/smartcard-sniffer/) log as input 
# and produce binary files in directory with time in seconds as name (i.e. 1436560907).
# File name is as 001_out(4)39_6A_42_32 in format:
# <# of sequence>_<in - command to card/out - data from card>(bytes in command/data)<first 4 bytes separated with underscore>
# This binary files can be easily seen by any hex editor....
#
#                      sergey v. soldatov, July 2015


my $j = 0;
my $d = time();

mkdir $d;
 
while(my $line = <STDIN>){
	my $fn = '';
	if($line =~ /^>>>/){ #request
		$line =~ s/^>>>\s+//; $line =~ s/\s+$//;
		$j++;
		@b_text = split /:/, $line;
		$fn = "$d\\".sprintf("%03d",$j)."_in(".@b_text.")".join("_",$b_text[0],$b_text[1],$b_text[2],$b_text[3]);
	}
	elsif($line =~ /^<<</){ #response
		$line =~ s/^<<<\s+//; $line =~ s/:?90:00\s+$//;
		@b_text = split /:/, $line;
		$fn = "$d\\".sprintf("%03d",$j)."_out(".@b_text.")".join("_",$b_text[0],$b_text[1],$b_text[2],$b_text[3]);	
	}
	else {
		next;
	}
	open OUT, '>:raw', $fn;
	print join(":",@b_text)."\n"; #DEBUG
	print OUT map( {pack 'C', hex($_) } @b_text);
	close OUT;
}
	