#!/usr/bin/perl
# WARNING: this code was written to work, nothing more nothing less :)

$size = 0;

($cb) = @ARGV;

print "struct debug_op debug_op_table";
print "_cb" unless $cb ne "cb";
print "[256] =\n{\n";

sub inst_parm
{
	($s) = @_;
	$r = "";
	
	$_ = $s;

	$size = 0;
	$data = 0;

	if($inst eq "RST")
	{
		return (int(substr($s,1)),0);
	}

	if($inst eq "CALL" || $inst eq "JP" || $inst eq "JR" || $inst eq "RET")
	{
		if($_ eq "NZ" || $_ eq "Z" || $_ eq "NC" || $_ eq "C")
		{
			return ($_,0);
		}
	}

	if($inst eq "SET" || $inst eq "BIT" || $inst eq "RES")
	{
		if(int($_) eq $_)
		{
			return ($s,0);
		}
	}
	
	if(/^\(.*\)$/)
	{
		$r .= "(";
		if(/^\(\$.*\)/)
		{
			$size++;	
			$r .= "\$%.2x";
			$r .= "%.2x" unless length() < 7;
			$size++ unless length() < 7;
		}
		else
		{
			$r .= substr($s,1,length($s)-2);	
		}
		$r .= ")";
	}
	elsif(/^\$.*$/)
	{
		$size++;
		$r .= "\$%.2x";
		$r .= "%.2x" unless length() < 5;
		$size++ unless length() < 5;
	}
	else
	{
		$r .= $s;	
	}
	return ($r,$size);
}

sub print_inst
{
	$i = "\"".uc($inst);
	$space_dummy = "     ";
	
	if(defined($p2))
	{
		($dummy,$s1) = inst_parm($p1);
		$i .= substr($space_dummy,0,5-length($inst));
		$i .= $dummy.",";
		
		($dummy,$s2) = inst_parm($p2);
		$i .= $dummy;
	}
	elsif(defined($p1))
	{
		($dummy,$s1) = inst_parm($p1);
		$i .= substr($space_dummy,0,5-length($inst));
		$i .= $dummy;
	}
	
	$i .= "\"";
	
	$dummy = "                     ";
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

	$leap = hex($opcode)-hex($last_opcode);
	
	if(hex($opcode)-$leap+1 eq 0xCB && $cb ne "cb")
	{
		print "\t{\"INTERNAL\",           0}, ";
		printf("/*0x%.2x*/ /* INTERNAL */\n",hex($opcode)-$leap+1);
		$count++;
	}
	elsif($leap > 1)
	{
		while($leap > 1)
		{
			print "\t{\"UNKNOWN\",            0}, ";
			printf("/*0x%.2x*/ /* UNKNOWN */\n",hex($opcode)-$leap+1);
			$leap--;
			$count++;
		}
	}
	$last_opcode = $opcode;

	print "\t{";
	
	$_ = $parms;
	($p1,$p2) = split(/,/);
	$s1 = $s2 = 0;

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
	
	if($inst eq "STOP")
	{
		print "1";
	}
	elsif(!defined($parms))
	{
		print "0";
	}
	else
	{	
		printf("%d",$s1+$s2);
	}	
	print "}";	
	print ", " unless $opcode eq "FF";
	print "  " unless $opcode ne "FF";
	print "/*0x";
	printf("%.2x",hex($opcode));
	print "*/\n";
	$count++;
}

print "\t/* Number of lines: $count */\n";
print "};\n";

