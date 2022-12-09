#!/usr/bin/perl -w
$err = 0;
$curFileName = "./curFile";
system("rm -rf OutPut; mkdir OutPut");

printf "\n============ Generate PQ BIN ============\n";

system("find . -name \"AML_PQ*.cpp\" > BinFiles");
open($file, "./BinFiles");
@lines = <$file>;
close $file;

foreach $oneFileName (@lines) {

        print "\ Start to Conversion $oneFileName";
        open($curFile, ">" , $curFileName);
        print $curFile "CUR_PQFILE=$oneFileName";
        close $curFile;

        system("rm -f GenPQFile");
        if (system("make FilePQ") == 0) {
            if (system("./GenPQFile") == 0) {
                $err++;
            }
        } else {
            printf "\033[31mMake fail when compile %s\033[m", $oneFileName;
            $err++;
        }
}

system("rm -f GenPQFile");

if ($err != 0) {
    printf("\033[31mFound $err error(s).\n");
}
exit $err;
