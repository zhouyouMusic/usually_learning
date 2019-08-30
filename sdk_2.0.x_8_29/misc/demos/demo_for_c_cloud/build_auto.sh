#! /bin/bash
cd ../../../
rm -rf skegn-*
bash build.sh mingw

bsPath="skegn-mingw-"
function getdir(){
    for element in `ls $1`
    do  
        dir_or_file=$1"/"$element
        if [ -d $dir_or_file ]
        then 
#            getdir $dir_or_file
#			echo "$dir_or_file"
			if [[ $dir_or_file == *$bsPath* ]]
			then	
				mv -f  $dir_or_file/*.dll  misc/demos/demo_for_c_cloud/skEgn.dll
				mv -f  $dir_or_file/*.lib  misc/demos/demo_for_c_cloud/skEgn.lib
				mv -f  $dir_or_file/*  misc/demos/demo_for_c_cloud/
			fi
        else
            continue
        fi  
    done
}
root_dir=$(cd `dirname $0`; pwd)
getdir $root_dir

cd misc/demos/demo_for_c_cloud


gcc -g demo_for_c.c skEgn.lib  -o test 

echo "All Done @@@@@@@@@@@@@@@@@@@"






