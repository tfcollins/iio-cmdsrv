#!/bin/sh
while [ true ]
do
	/bin/nc -l -l -p 1234 -e /usr/local/bin/iio_cmdsrv 
done
