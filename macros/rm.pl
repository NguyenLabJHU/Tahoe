#!/usr/bin/perl -w
# $Id: rm.pl,v 1.1 2004-08-10 01:03:04 paklein Exp $
if (scalar(@ARGV) < 1 || $ARGV[0] =~ /-h/) {
    print STDOUT "\tusage : rm.pl [files or file patterns to remove]\n";
    exit 0;
}

for ($i = 0; $i < scalar(@ARGV); $i++) {

    # set up search pattern
    $pattern = $ARGV[$i];
    
    # make '.' literal
    $pattern =~ s/\./[\.]/g;

    # correct wild cards
    $pattern =~ s/\*/\.\*/g;

    # report
    print STDOUT "regex pattern = \"$pattern\"\n";

    # run through files in the current directory
    open(LS, "ls -1 |");
    $count = 0;
    while (defined($file = <LS>)) {
	chomp($file);
	if ($file =~ /^${pattern}$/) {
	    system("rm $file");
	    $count++;
	}
    }
    
    # report
    print STDOUT "removed $count files\n";

    close(LS);
}

exit 1;
