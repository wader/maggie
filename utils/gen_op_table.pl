#!/usr/bin/perl
# WARNING: this code was written to work, nothing more nothing less :)

%functions = ();

$size = 0;

($mode,$cb) = @ARGV;

if($mode eq "struct")
{
	print "struct cpu_op cpu_op_table";
	print "_cb" unless $cb ne "cb";
	print "[256] =\n{\n";
}

sub inst_parm
{
	($s) = @_;
	$r = "";
	
	$_ = $s;

	$size = 0;
	$data = 0;

	if($inst eq "RST")
	{
		return ("",0,"0x".substr($s,1));
	}

	if($inst eq "CALL" || $inst eq "JP" || $inst eq "JR" || $inst eq "RET")
	{
		if($_ eq "NZ" || $_ eq "Z" || $_ eq "NC" || $_ eq "C")
		{
			return ("_cond",0,"COND_$s");
		}
	}

	if($inst eq "SET" || $inst eq "BIT" || $inst eq "RES")
	{
		if(int($_) eq $_)
		{
			return ("_cons",0,"0x0".$s);
		}
	}
	
	if(/^\(.*\)$/)
	{
		$r .= "_ind";
		if(/^\(\$.*\)/)
		{
			$size++;	
			$r .= "imm";
			$r .= "16" unless length() < 7;
			$size++ unless length() < 7;
		}
		else
		{
			$r .= "reg";
			$r .= "16" unless length() < 4;
			$data = "REG_".substr($s,1,length($s)-2);
		}
	}
	elsif(/^\$.*$/)
	{
		$size++;
		$r .= "_imm";
		$r .= "16" unless length() < 5;
		$size++ unless length() < 5;
	}
	else
	{
		$r .= "_reg"; 
		$r .= "16" unless length() < 2;
		$data = "REG_$s";
	}
	return ($r,$size,$data);
}

sub print_inst
{
	$i = "cpu_op_";
	$i .= lc($inst);
	
	if(defined($p2))
	{
		($dummy,$s1,$d1) = inst_parm($p1);
		$i .= $dummy;
		
		($dummy,$s2,$d2) = inst_parm($p2);
		$i .= $dummy;
	}
	elsif(defined($p1))
	{
		($dummy,$s1,$d1) = inst_parm($p1);
		$i .= $dummy;
	}
	
	$functions{$i} .= "$inst";
	$functions{$i} .= " $p1" unless !defined($p1);
	$functions{$i} .= ",$p2" unless !defined($p2);
	$functions{$i} .= "  ";
	
	return unless $mode eq "struct";
	
	$dummy = "                         ";

	$dummy = substr($dummy,0,length($dummy)-length($i));
	$dummy = $i.",".$dummy;
	print $dummy;	
}

while(<STDIN>)
{
	$_ = substr($_,3) unless $cb ne "cb";
	
	chomp;
	s/ xx//;
	s/ bb//;
	s/ aa//;
	s/ 00//;
	s/\r//;
	split(/ /);
	($opcode,$inst,$parms,$clock) = @_;

	if($mode eq "struct" && defined($last_opcode))
	{
		$leap = hex($opcode)-hex($last_opcode);

		if(hex($opcode)-$leap+1 eq 0xCB && $cb ne "cb")
		{
			printf("\t{cpu_op_cb,                0,\t0,\t0,\t0,\t0},\t/*0xcb*/ /* INTERNAL */\n");
			$count++;
		}
		elsif($leap > 1)
		{
			while($leap > 1)
			{
				printf("\t{cpu_op_unknown,           0,\t0x%.2x,\t0,\t0,\t0},\t",hex($opcode)-$leap+1);
				printf("/*0x%.2x*/ /* UNKNOWN */\n",hex($opcode)-$leap+1);
				$leap--;
				$count++;
			}
		}
	}
	$last_opcode = $opcode;

	print "\t{" unless $mode ne "struct";
	
	$_ = $parms;
	($p1,$p2) = split(/,/);
	$s1 = $s2 = 0;
	$d1 = $d2 = 0;

	if(int($parms) eq $parms)
	{
		$clock = $parms;
		undef($parms);
	}

	if(!defined($p2))
	{	
		$p1 = $parms;
	}

	print_inst();

	next unless $mode eq "struct";
	
	if(!defined($parms))
	{
		if($inst eq "STOP")
		{
			print "1,\t0,\t0,\t0,\t$clock";
		}
		else
		{
			print "0,\t0,\t0,\t0,\t$clock";
		}
	}
	else
	{
		$d1 .= ",";
		$d2 .= ",";
		if(length($d1) < 8)
		{
			$d1 .= "\t";
		}
		if(length($d2) < 8)
		{
			$d2 .= "\t";
		}
	
		printf("%d,\t%s%d,\t%s%d",$s1,$d1,$s2,$d2,$clock);
	}	
	print "}";	
	print "," unless $opcode eq "FF";
	print " " unless $opcode ne "FF";
	print "\t/*0x";
	printf("%.2x",hex($opcode));
	print "*/\n";	
	$count++;
}
if($mode eq "struct")
{
	print "\t/* Number of lines: $count */\n";
	print "};\n";
}

sort(keys(%functions));

if($mode eq "header")
{
	foreach $key (sort(keys(%functions)))
	{
		print "void $key();\n";
	}
}

if($mode eq "func")
{
	foreach $key (sort(keys(%functions)))
	{
		print "// ".substr($functions{$key},0,length($functions{$key})-2)."\n";
		print "void $key()\n";
		print "{\n";
		print "}\n\n";
	}
}


