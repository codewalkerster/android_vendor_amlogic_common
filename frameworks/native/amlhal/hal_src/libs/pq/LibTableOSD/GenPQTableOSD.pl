#!/usr/bin/perl -w
$err = 0;
$curFileName = "./curFile";
system("rm -rf OutPut; mkdir OutPut");

printf "\n============ Generate PQ OSD BIN ============\n";

system("find . -name \"AML_PQ_OSD*.cpp\" > BinFiles");
open($file, "./BinFiles");
@lines = <$file>;
close $file;

foreach $oneFileName (@lines) {

	print "\ Start to Conversion $oneFileName";
	open($curFile, ">" , $curFileName);
	print $curFile "CUR_PQOSDFILE=$oneFileName";
	close $curFile;

	system("rm -f GenPQOsdFile");
        if (system("make FilePQOSD") == 0) {
                if (system("./GenPQOsdFile") == 0) {
			$err++;
		}
        } else {
		printf "\033[31mMake fail when compile %s\033[m", $oneFileName;
		$err++;
	}
}

system("rm -f GenPQOsdFile");

if ($err != 0) {
	printf("\033[31mFound $err error(s).\n");
}
exit $err;
