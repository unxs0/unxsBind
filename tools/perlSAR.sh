#!/bin/sh
#
#FILE
# perlSAR.sh
#PURPOSE
# Automate porting to RAD3 style for existing tTABLEfunc.h files
# where tTABLE.c files built from new RAD3 templates
#AUTHOR
# (c) 2006 gary wallis
#

#Things to change
#
#mode to guMode
#function to gcFunction
#find to gcFind
#filter to gcFilter
#uReseller to guReseller
#uPermLevel to guPermLevel
#uLoginClient to guLoginClient
#command to gcCommand
#mysql to gMysql
#query to gcQuery

#patch 1
for file in $( ls -1 t*func.h ); do

	file_check=`grep -l "perlSAR patch1" $file`

	if [ "$file_check" = "$file"  ]; then

		echo NOT editing already edited file: $file ...

	else

		echo editing $file ...

		/usr/bin/perl -pi -e "s/mode/guMode/g" $file
		/usr/bin/perl -pi -e "s/function/gcFunction/g" $file
		/usr/bin/perl -pi -e "s/find/gcFind/g" $file
		/usr/bin/perl -pi -e "s/filter/gcFilter/g" $file
		/usr/bin/perl -pi -e "s/uReseller/guReseller/g" $file
		/usr/bin/perl -pi -e "s/uPermLevel/guPermLevel/g" $file
		/usr/bin/perl -pi -e "s/uLoginClient/guLoginClient/g" $file
		/usr/bin/perl -pi -e "s/command/gcCommand/g" $file
		/usr/bin/perl -pi -e "s/\&mysql/\&gMysql/g" $file
		/usr/bin/perl -pi -e "s/,query/,gcQuery/g" $file
		/usr/bin/perl -pi -e "s/\(query\)/\(gcQuery\)/g" $file
		/usr/bin/perl -pi -e "s/query,/gcQuery,/g" $file
		echo "//perlSAR patch1" >> $file

	fi
done

#patch 2
for file in $( ls -1 bind.c ); do

	file_check=`grep -l "perlSAR patch2" $file`

	if [ "$file_check" = "$file"  ]; then

		echo NOT editing already edited file: $file ...

	else

		echo editing $file ...

		/usr/bin/perl -pi -e "s/\&mysql/\&gMysql/g" $file
		/usr/bin/perl -pi -e "s/,query/,gcQuery/g" $file
		/usr/bin/perl -pi -e "s/\(query\)/\(gcQuery\)/g" $file
		/usr/bin/perl -pi -e "s/query,/gcQuery,/g" $file
		echo "//perlSAR patch2" >> $file

	fi
done
