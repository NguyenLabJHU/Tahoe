#!/usr/bin/perl
# $Id: regression_trigger.pl,v 1.1 2003-04-01 18:43:39 paklein Exp $

# configurations that must be defines
$TAHOE_USER = "paklein";
$HOST = "DARWIN-3.3.LAM";

# configurations that should not need changes
$TAHOE_HOST = "tahoe.ca.sandia.gov";
$TRIGGER_PATH = "/outgoing";
$TRIGGER_FILE = "run_test";
$STATUS_PATH = "/incoming/benchmark/status";
$THE_SCRIPT = "regression_trigger.pl";
$MESG_FILE = $THE_SCRIPT;
$MESG_FILE =~ s/pl$/msg/;
$LOCK_FILE = $THE_SCRIPT;
$LOCK_FILE =~ s/pl$/lock/;
$BENCHMARK_SCRIPT = "dummy.csh";

# check if this script is already running
if (-e $LOCK_FILE) 
{
	printf "lock file exists: $LOCK_FILE\n";
	exit;
} 
else
{
	# open lock file
	open(LOCK, ">$LOCK_FILE") || die "could not open file: $LOCK_FILE\n";

	# get the date
	($sec, $min, $hour, $mday, $mon, $year, $wday, $yday, $isdst) = localtime;
	$the_date = sprintf("%02d%02d%04d-%02d:%02d:%02d", $mon+1, $mday, $year+1900, $hour, $min, $sec);

	# mark file
	print LOCK "$THE_SCRIPT is running: $the_date\n";
}

# open message file
open(MESG, ">$MESG_FILE") || die "could not open file: $MESG_FILE\n";
print MESG "start: $the_date\n";

# get new trigger file from the Tahoe server
system("scp $TAHOE_USER\@$TAHOE_HOST:$TRIGGER_PATH/$TRIGGER_FILE $TRIGGER_FILE");

# check trigger time stamp
if (-e $TRIGGER_FILE)
{
	# last trigger file
	$last_file = $TRIGGER_FILE."_last";

	$run_test = 0;
	if (! -e $last_file)
	{
		print MESG "no last trigger file: $last_file\n";
		$run_test = 1; 
	}
	else # compare time stamps in new and last trigger file
	{
		# read dates
		open(LAST, $last_file) || die "could not open file: $last_file\n";
		$last_date = <LAST>;
		chomp($last_date);

		open(NEW, $TRIGGER_FILE) || die "could not open file: $TRIGGER_FILE\n";
		$next_date = <NEW>;
		chomp($next_date);
		
		# compare date strings
		if ($next_date =~ /$last_date/)
		{
			print MESG "tests are up to date\n";
			$run_test = 0;
		}
		else
		{
			print MESG "tests are out of date:\n\told: $last_date\n\tnew: $next_date\n";
			$run_test = 1;
		}
	}

	# run the test
	if ($run_test == 1)
	{
		# send status to server
		open(STATUS, ">status.tmp");
		print STATUS "$HOST\n";
		print STATUS "$next_date\n";
		print STATUS "RUNNING\n";
		close(STATUS);
		system("scp status.tmp $TAHOE_USER\@$TAHOE_HOST:$STATUS_PATH/$HOST");

		print MESG "running tests...\n";
		system("csh -f $BENCHMARK_SCRIPT");
		print MESG "tests done\n";

		# send status to server
		open(STATUS, ">status.tmp");
		print STATUS "$HOST\n";
		print STATUS "$next_date\n";
		print STATUS "DONE\n";
		close(STATUS);
		system("scp status.tmp $TAHOE_USER\@$TAHOE_HOST:$STATUS_PATH/$HOST");
	}

	# store file
	system("mv $TRIGGER_FILE $last_file");
}
else
{
	print MESG "could not find trigger file: $TRIGGER_FILE\n";
}

# report the stop time
($sec, $min, $hour, $mday, $mon, $year, $wday, $yday, $isdst) = localtime;
$the_date = sprintf("%02d%02d%04d-%02d:%02d:%02d", $mon+1, $mday, $year+1900, $hour, $min, $sec);
print MESG "stop: $the_date\n";

# clean up
system("rm $LOCK_FILE");
