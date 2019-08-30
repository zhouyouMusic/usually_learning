#!/bin/sh

workd=/home/listen/codingDir/HTTPS_JSON_PRJ/cjson_8098_alsa_default/
fileName=monitor_zhou
fileNameOld=$fileName"old"
fileNameNew=$fileName"new"

bFileValid=0
bFileOldValid=0
bFileNewValid=0

#检查文件是否可执行
checkFileExecutable()
{
    #echo "check file $1 executable"
    local ret=`file $1 | grep executable | grep -v grep | wc -l`
    return $ret;
}

#检查文件是否有效，先检查是否存在，再检查是否可执行
chechFileValid()
{
    local valid
    if [ -f $1 ]; then
        #echo "file $1 exist"

        checkFileExecutable $1
        valid=$?
        if [ $valid -eq 1 ] ;  then
            #echo "file $1 is executable"
            valid=1
        else
            #echo "file $1 is note executable, remove it"
            rm $1
        fi

    else
        #echo "file $1 not exist"
        valid=0;
    fi

    return $valid;
}

chechFileValid $fileName
bFileValid=$?
chechFileValid $fileNameNew
bFileNewValid=$?
chechFileValid $fileNameOld
bFileOldValid=$?

#echo $bFileValid $bFileNewValid $bFileOldValid

while [ true ] ; do
    processExist=`ps -ef | grep $fileName | grep -v grep | wc -l`
    if [ $processExist -gt 0 ] ; then
        sleep 2;
    else
        chechFileValid $fileName
        bFileValid=$?
        chechFileValid $fileNameNew
        bFileNewValid=$?
        chechFileValid $fileNameOld
        bFileOldValid=$?

        if [ $bFileNewValid -eq 1 ] ; then
            #echo "new file valid"
            if [ $bFileValid -eq 1 ] ; then
                if [ $bFileOldValid -eq 1 ] ; then
                    #echo "file old found, remove it"
                    rm $fileNameOld
                fi
                #echo "current file valid ,rename it to old"
                mv $fileName $fileNameOld
            fi
            #echo "new file valid ,rename it to current"
            mv $fileNameNew $fileName
        else
            if [ $bFileValid -eq 1 ] ; then
                if [ $bFileOldValid -eq 1 ] ; then
                    #echo "file old found, remove it"
                    rm $fileNameOld
                fi
            else
                #echo "rename old to current"
                mv $fileNameOld $fileName
            fi
        fi
        sync
        $workd$fileName
        sleep 1;
    fi
done;
