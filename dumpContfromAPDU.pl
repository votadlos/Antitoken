#!/usr/bin/perl
#
# Dump Crypto-Pro CSP's container from APDU
#
# Take Smartcard Sniffer's (https://code.google.com/p/smartcard-sniffer/) log as input 
# and produce key container export in CryptoPRO format:
# <DIR>/header.key
# <DIR>/name.key
# <DIR>/masks.key
# <DIR>/primary.key
#
# works on eToken PRO Java 72K OS755(eToken Java Applet 1.1.25)
#
#                      sergey v. soldatov, July 2014

my %export = ();
my $preambula = '00:A4:08:04:0C:66:66:50:00:E0:0E:0B:00:CC';
my $prefix = '80:18:00:00:04:0E';

while(my $req = <STDIN>){
	my $resp = <STDIN>; #get card answer
	$resp =~ s/:90:00\s*$//;
	$resp =~ s/^<<<\s+//;
	my @bytes = split /:/, $resp;

	if ($req =~ /^>>> $preambula:(\w{2}):F0:03:00/){ #Prepare to receive header.key
		$export{container_id} = $1;
		$export{header}->{ready_for_header} = 1;
	}
	elsif($req =~ /^>>> $prefix:02:00:00:0A/ && $export{header}->{ready_for_header} == 1){ #first 10 bytes of header
		push @{$export{header}->{data}}, map( {pack 'C', hex($_) } @bytes);
		my ($b2, $b3) = map {hex($_)} ($bytes[2],$bytes[3]);
		$export{header}->{header_len} = ($b2 << 8) + $b3;
		$export{header}->{bytes_received} = 10;
		$export{header}->{ready_for_header} = 0;
		#print "Header len=".$export{header}->{header_len}." b2 = $b2 ($bytes[2]), b3 = $b3 ($bytes[3])\n";
	}
	elsif($req =~ /^>>> $prefix:/ && $export{header}->{bytes_received} <  $export{header}->{header_len}){ #rest header data 
		push @{$export{header}->{data}}, map( {pack 'C', hex($_) } @bytes);
		$req =~ s/^>>>\s+//; my @req_bytes = split /:/, $req;
		$export{header}->{bytes_received} += hex($req_bytes[9]);
		#print "bytes_received=".$export{header}->{bytes_received}."\n";
	}
	elsif($req =~ /^>>> 80:11:00:11:/ && $resp =~ /90:00/){ #auth OK
		$export{auth} = 1;
		$export{header}->{bytes_received} = $export{header}->{header_len}; #stop collect header!
		#print $export{auth}."\n";
	}
	elsif ($export{auth} && $req =~ /^>>> $preambula:$export{container_id}:F0:01:00/){ #Prepare to receive masks.key
		$export{masks}->{ready_for_masks} = 1;
		#print $export{masks}->{ready_for_masks}."\n".$export{container_id}."\n";
	}
	elsif($req =~ /^>>> $prefix:02:00:00:38/ && $export{masks}->{ready_for_masks} && $resp =~ /^30:36/){ #masks.key
		push @{$export{masks}->{data}}, map( {pack 'C', hex($_) } @bytes);
		$export{masks}->{ready_for_masks} = 0;
	}
	elsif($export{auth} && $req =~ /^>>> $preambula:$export{container_id}:F0:02:00/){ #Prepare to recive primary.key
		$export{primary}->{ready_for_primary} = 1;
	}
	elsif($req =~ /^>>> $prefix:02:00:00:24/ && $export{primary}->{ready_for_primary} && $resp =~ /^30:22/){ #primary.key
		push @{$export{primary}->{data}}, map( {pack 'C', hex($_) } @bytes);
		$export{primary}->{ready_for_primary} = 0;
	}
}

my $suf = time();
mkdir "./$suf";
foreach my $fn('header','primary','masks'){
	open ($out, '>:raw', "./$suf/$fn.key") or die "Unable to open ./$suf/$fn.key: $!\n";
	print $out @{$export{$fn}->{data}};
	close $out;	
}

open ($out, '>:raw', "./$suf/name.key") or die "Unable to open ./$suf/name.key: $!\n";
print $out map( {pack 'C', hex($_) } ('30','13','16','11') );
print $out "ex_$suf";
close $out;
