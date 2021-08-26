#!/bin/bash

DIR=$(dirname "$1")
if [ ! -d "$DIR" ]; then
	mkdir "$DIR"
fi

i=0
for var in "$@"
do
	((i++))
    if [[ "$i" == '1' ]]; then
		continue
  	fi
	
	echo $var $1
	
	cp -r $var/. $1/
done