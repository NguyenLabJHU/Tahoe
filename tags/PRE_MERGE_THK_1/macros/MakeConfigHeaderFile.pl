#!/usr/bin/perl -w
#
# $Id: MakeConfigHeaderFile.pl,v 1.5 2003-01-30 00:46:38 paklein Exp $
#
# Generates a C/C++ header file from a configuration file which is
# passed as the command-line argument.
#
# The format of the file fed to this script is:
#
# [root of output file names]
# 
# [root of preprocessor symbol for option 1]
# [ENABLE/DISABLE for option 1]
# [one line description for option 1]
# [source directories for option 1]
#
# [root of preprocessor symbol for option 2]
# [ENABLE/DISABLE for option 2]
# [one line description for option 2]
# [source directories for option 2]
#
# ...and so one with three lines of information for each option
#

if (scalar(@ARGV) == 0) {
	print STDOUT "\tusage: MakeConfigHeaderFile.pl [config file] [(optional) output directory]\n";
	exit;
}

$config_file = $ARGV[0];
$destination = ".";
if (scalar(@ARGV) > 1) { $destination = $ARGV[1]; }

print "reading configuration file: $config_file\n";

# skip leading comments
open(IN, $config_file) || die "could not open config file: $config_file\n";
$out_root = <IN>;
chomp($out_root);
while ($out_root =~ /^#/ || $out_root !~ /[a-zA-Z0-9]/ ) {
	$out_root = <IN>;
	chomp($out_root);
}

# output file
print "output file root: $out_root\n";
$config_header_file = "$destination" . "/" . $out_root . ".h";
if (-e $config_header_file) {
	print "output file already exists: $config_header_file\n";
	print "stop.\n";
	exit;
}
else {
	print "creating output file: $config_header_file\n";
}
open(OUT, ">$config_header_file") || die "could not open output file: $config_header_file\n";

# create pre-processor macro name
$pre_pro_macro = $out_root."H_";
$pre_pro_macro =~ s/([A-Z])/\_$1/g;
$pre_pro_macro = uc($pre_pro_macro);

# print header
print OUT <<FIN;
/* \$Id\$ */
#ifndef $pre_pro_macro
#define $pre_pro_macro
/* This file was generated by MakeConfigHeaderFile.pl from $config_file */
FIN

open (DATE, "date |");
while (<DATE>) {
	chomp($_);
	print OUT "/* created: " . $_ . " */\n";
}
close(DATE);

print OUT <<FIN;

/** \\file $out_root.h
 * Configuration of optional components within Tahoe.
 * Sections of the code are included or excluded in the build of Tahoe depending in 
 * this flags in this file and in the file $out_root.make. Each option has
 * a #define definition in this file and a corresponding directory definition
 * in $out_root.make. The two items must be set consistently to enable or
 * disable materials models. To enable an option:
 * -# in this file, uncomment the #define statement
 * -# in $out_root.make, uncomment the macro.
 *
 * The naming convention for the definitions in this file and the macros in
 * $out_root.make are as follows. For the option [OPTION]:
 * -# the symbol in this file will be [OPTION]
 * -# the macro defining the corresponding source directory will be DIRECTORY_[OPTION]
 */
FIN

# scan file for options
$scan_line = 1;
$opt_root = "";
$enabled = 1;
while ($line = <IN>) {
	chomp($line);
	
	# non-comment, non-blank line
	if ($line !~ /^#/ && $line =~ /[a-zA-Z0-9]/ ) {
	
		# read name
		if ($scan_line == 1) {
			$opt_root = $line;
			print OUT "\n/** \\def $opt_root\n";
			print "processing option: $opt_root\n";
			$scan_line++;
		} 
		# read ENABLE/DISABLE
		elsif ($scan_line == 2) {
		
			if ($line =~ /DISABLE/) { 
				$enabled = 0; 
			} else { 
				$enabled = 1; 
			}
			$scan_line++;
		} 
		# read description
		elsif ($scan_line == 3) {
		
			# add period
			if ($line !~ /\.$/) { $line = $line."."; }

			print OUT " * $line\n";
			print OUT " * This option must be set in conjunction with the DIRECTORY_$opt_root macro\n";
			print OUT " * in $out_root.make. */\n";
			if ($enabled == 1) {
				print OUT "#define $opt_root 1\n";
			} else {
				print OUT "/* #define $opt_root 1 */\n";
			}
			$scan_line++;
		}
		# read directories
		else {		
			$scan_line = 1;
		}
	}
}

# print footer
print OUT "\n#endif\n";