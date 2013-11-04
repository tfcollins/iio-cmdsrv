#!/bin/sh

count=0
p=$1

if [ "$#" -eq "0" ] ; then
	p=0;
fi

while ./lib_iio_cmdsrv 192.168.1.114 > /dev/null
do
	count=$((count+1))
	sleep $p
	echo $count
done
echo $count
