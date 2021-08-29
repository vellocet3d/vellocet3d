#!/bin/bash

#https://stackoverflow.com/Questions/3821916/how-to-merge-two-ar-static-libraries-into-one


DIR=$(dirname "$1")
if [ ! -d "$DIR" ]; then
	mkdir "$DIR"
fi

CMD="ar cqT ${1}"

i=0
for var in "$@"
do
	((i++))
    if [[ "$i" == '1' ]]; then
		continue
  	fi
	
	CMD="${CMD} ${var}"
done

if [ -f "$1" ] ; then
    rm "$1"
fi

eval $CMD
echo -e "create ${1}\naddlib ${1}\nsave\nend" | ar -M