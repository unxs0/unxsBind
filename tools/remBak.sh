#!/bin/bash
#
#Purpose
#	Specific file renamer for bind.tgz files from Globix
#Use
#	first 
#	tar -xzf bind.tgz -C /usr/local/idns
#
cd /usr/local/idns/bind/zoneFiles;
if [ $? -ne 0 ]; then
	exit 1;
fi
mkdir /usr/local/idns/import/;

for cFile in $( ls -1 *.bak ); do

	cNewName=`echo "$cFile" | sed -e s/\.bak//g`;
	if [ $? -ne 0 ]; then
		exit 2;
	fi

	#Some feedback
	echo "$cFile $cNewName";

	cp $cFile ../../import/$cNewName;
	if [ $? -ne 0 ]; then
		exit 2;
	fi
done
chmod -R go+r /usr/local/idns/import/
cd /usr/local/idns/bind;

