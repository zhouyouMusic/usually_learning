#!/bin/sh  
while true
do 
	filename=mj.log  
	filesize=`ls -l $filename | awk '{ print $5 }'`  
	maxsize=$((1024 * 10))  
	if [ $filesize -gt $maxsize ]  
	then  
#	    echo "$filesize > $maxsize"  
	    echo "" > $filename
#	else   
#	    echo "$filesize < $maxsize"  
	fi 
	sleep 3
done 
