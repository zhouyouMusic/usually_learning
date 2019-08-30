#! /bin/bash

function try_catch()
{
    if [[ $1 != 0 ]]; then
        echo catch exception
        exit $1
    fi
}

function get_externals()
{
    svn info |awk '($1=="URL:"){print $2}' > url.txt; try_catch $?;
    svn propget svn:externals >> url.txt; try_catch $?;
    svn info |awk '($1=="Revision:"||$1=="版本:") {print $2}' > revision.txt;try_catch $?;
    svn propget svn:externals |grep 'http\S*' -o |xargs svn info --username demo --password demo|awk '($1=="Revision:"||$1=="版本:") {print $2}' >> revision.txt; sed -ig 's/^/-r /g' revision.txt;try_catch $?;
    paste -d " " revision.txt url.txt > svn.revision;try_catch $?;
    #rm url.txt revision.txt;
}

function build_for_ios()
{
	# ar -x opus
	#cd third/opus_libs/iOS/armv7 && ar -x *.a && cd -
	#cd third/opus_libs/iOS/armv7s && ar -x *.a && cd -
	#cd third/opus_libs/iOS/arm64 && ar -x *.a && cd -
	#cd third/opus_libs/iOS/i386 && ar -x *.a && cd -
	#cd third/opus_libs/iOS/x86_64 && ar -x *.a && cd -
	# ar -x nativecore
    cd third/native_corelib/ios/armv7 && ar -x *.a && cd -
    cd third/native_corelib/ios/armv7s && ar -x *.a && cd -
    cd third/native_corelib/ios/arm64 && ar -x *.a && cd -
    cd third/native_corelib/ios/i386 && ar -x *.a && cd -
    cd third/native_corelib/ios/x86_64 && ar -x *.a && cd -
    # provision
    VERSION=`awk -F\" '/SKEGN_VERSION/{print $$2}' include/skegn.h`
	OPT_CFLAGS="-Ofast -flto -g"
    OPT_LDFLAGS="-flto"
    OPT_CONFIG_ARGS=""
	
	ARCHS="i386 x86_64 armv7 armv7s arm64"
	SERVERS="native cloud"
	DEVELOPER=`xcode-select -print-path`
	
	for SERVER in ${SERVERS}
	do
    	for ARCH in ${ARCHS}
        do
            if [ "${ARCH}" == "i386" ] || [ "${ARCH}" == "x86_64" ]; then
                PLATFORM="iPhoneSimulator"
                EXTRA_CFLAGS="-arch ${ARCH}"
                EXTRA_CONFIG="--host=x86_64-apple-darwin"
            else
                PLATFORM="iPhoneOS"
                EXTRA_CFLAGS="-arch ${ARCH}"
                EXTRA_CONFIG="--host=arm-apple-darwin"
            fi
		make -f Makefile-iOS clean all TIMESTAMP=$TIMESTAMP ARCH=$ARCH DEVELOPER=$DEVELOPER PLATFORM=$PLATFORM SERVER=$SERVER; try_catch $?;
#		make -f Makefile-iOS clean all TIMESTAMP=$TIMESTAMP ARCH=$ARCH DEVELOPER=$DEVELOPER PLATFORM=$PLATFORM SERVER=$SERVER  USE_SSL=1; try_catch $?;
    	done
        xcrun -sdk iphoneos lipo -output libskegn-ios-$SERVER-$VERSION-$TIMESTAMP.a -create -arch armv7 libskegn-armv7-$TIMESTAMP.a \
        -arch armv7s libskegn-armv7s-$TIMESTAMP.a -arch arm64 libskegn-arm64-$TIMESTAMP.a \
        -arch i386 libskegn-i386-$TIMESTAMP.a  -arch x86_64 libskegn-x86_64-$TIMESTAMP.a 
		
		xcrun -sdk iphoneos lipo -output libskegn-ssl-ios-$SERVER-$VERSION-$TIMESTAMP.a -create -arch armv7 libskegn-ssl-armv7-$TIMESTAMP.a \
        -arch armv7s libskegn-ssl-armv7s-$TIMESTAMP.a -arch arm64 libskegn-ssl-arm64-$TIMESTAMP.a \
        -arch i386 libskegn-ssl-i386-$TIMESTAMP.a  -arch x86_64 libskegn-ssl-x86_64-$TIMESTAMP.a 
    done
    TMPDIR=skegn-ios-$VERSION-$TIMESTAMP
    rm -rf $TMPDIR; mkdir $TMPDIR
    rm -f libskegn-*-ios-$VERSION-$TIMESTAMP.armv7.a	libskegn-ssl-*-ios-$VERSION-$TIMESTAMP.armv7.a 
    rm -f libskegn-*-ios-$VERSION-$TIMESTAMP.armv7s.a	libskegn-ssl-*-ios-$VERSION-$TIMESTAMP.armv7s.a  
    rm -f libskegn-*-ios-$VERSION-$TIMESTAMP.armv8.a	libskegn-ssl-*-ios-$VERSION-$TIMESTAMP.armv8.a
    rm -f libskegn-*-ios-$VERSION-$TIMESTAMP.i386.a    libskegn-ssl-*-ios-$VERSION-$TIMESTAMP.i386.a
    rm -f libskegn-*-ios-$VERSION-$TIMESTAMP.x86_64.a	libskegn-ssl-*-ios-$VERSION-$TIMESTAMP.x86_64.a
    mv third/opus_libs/iOS_1.3.1/libopus.a $TMPDIR
    mv libskegn-ios-cloud-$VERSION-$TIMESTAMP.a    libskegn-ssl-ios-cloud-$VERSION-$TIMESTAMP.a $TMPDIR
    mv libskegn-ios-native-$VERSION-$TIMESTAMP.a    libskegn-ssl-ios-native-$VERSION-$TIMESTAMP.a $TMPDIR
    cp include/skegn.h $TMPDIR
    #rm -f third/opus_libs/iOS/armv7/*.o
    #rm -f third/opus_libs/iOS/armv7s/*.o
    #rm -f third/opus_libs/iOS/arm64/*.o
    #rm -f third/opus_libs/iOS/i386/*.o
    #rm -f third/opus_libs/iOS/x86_64/*.o
    
    rm -f third/native_corelib/ios/armv7/*.o
    rm -f third/native_corelib/ios/armv7s/*.o
    rm -f third/native_corelib/ios/arm64/*.o
    rm -f third/native_corelib/ios/i386/*.o
    rm -f third/native_corelib/ios/x86_64/*.o
    #cp svn.revision $TMPDIR

    tar czf skegn-ios-$VERSION-$TIMESTAMP.tar.gz $TMPDIR
    return 0
}


function move_ssl_library()
{
	root_dir=$(cd `dirname $0`; pwd)
	cd $1
	for element in `ls`
	do  
        dir_or_file=$element
        if [ -d $dir_or_file ]
		
        then 
			if [[ $dir_or_file == "arm64-v8a" ]]
			then
				\cp -f ../third/openssl/libs/android/aarch64/lib*.so $dir_or_file/	
			fi
			
			if [[ $dir_or_file == "armeabi" ]]
			then
				\cp -f ../third/openssl/libs/android/armv6-vfp/lib*.so $dir_or_file/	
			fi
			
			if [[ $dir_or_file == "armeabi-v7a" ]]
			then
				\cp -f ../third/openssl/libs/android/armv7a-neon/lib*.so $dir_or_file/	
			fi
			
			if [[ $dir_or_file == "mips" ]]
			then
				\cp -f ../third/openssl/libs/android/mips/lib*.so $dir_or_file/	
			fi
			
			if [[ $dir_or_file == "mips64" ]]
			then
				\cp -f ../third/openssl/libs/android/mips64/lib*.so $dir_or_file/	
			fi
			
			if [[ $dir_or_file == "x86" ]]
			then
				\cp -f ../third/openssl/libs/android/i686/lib*.so $dir_or_file/	
			fi
			
			if [[ $dir_or_file == "x86_64" ]]
			then
				\cp -f ../third/openssl/libs/android/x86_64/lib*.so $dir_or_file/	
			fi
        else
            continue
        fi  
    done
	cd $root_dir
}

function build_for_android()
{
	SYS=`uname`
	if [[ $SYS != "Linux" && $SYS != "Darwin" ]]; then
		SYS=".cmd"
	else
		SYS=""
	fi
    # cloud
    cd jni/ && /opt/android-ndk-r13b/ndk-build$SYS clean all&& cd -
	mv libs cloud
	
	# cloud ssl
#	cd jni/ && /opt/android-ndk-r13b/ndk-build$SYS USE_SSL=1 clean all&& cd -
 	mv libs cloud_ssl
 	move_ssl_library cloud_ssl
	
	# native
	cd jni/ && /opt/android-ndk-r13b/ndk-build$SYS USE_NATIVE=1 LOCAL_ALLOW_UNDEFINED_SYMBOLS=true clean all && cd -
	mv libs native
	
	# native ssl
#	cd jni/ && /opt/android-ndk-r13b/ndk-build$SYS USE_NATIVE=1 USE_SSL=1 LOCAL_ALLOW_UNDEFINED_SYMBOLS=true clean all && cd -
 	mv libs native_ssl
 	move_ssl_library native_ssl

    # package
    TMPDIR=skegn-android-$VERSION-$TIMESTAMP
    rm -rf $TMPDIR; mkdir $TMPDIR
    #mv skegn-*-$TIMESTAMP.sh $TMPDIR
    mv native cloud native_ssl cloud_ssl $TMPDIR
    cp include/SkEgn.java $TMPDIR
    #cp svn.revision $TMPDIR

    tar czf skegn-android-$VERSION-$TIMESTAMP.tar.gz $TMPDIR
    return 0
}


function build_for_linux()
{
    # provison
    make -f Makefile-linux clean all TIMESTAMP=$TIMESTAMP ; try_catch $?;
    #ssl
#    make -f Makefile-linux clean all TIMESTAMP=$TIMESTAMP USE_SSL=1; try_catch $?;
    # package
    TMPDIR=skegn-linux-$VERSION-$TIMESTAMP
    rm -rf $TMPDIR; mkdir $TMPDIR
    #mv skegn-*-$TIMESTAMP.sh $TMPDIR
    mv libskegn-*-$TIMESTAMP.so $TMPDIR
    cp include/skegn.h include/SkEgn.java $TMPDIR
 
    cp third/openssl/libs/linux/lib*.so $TMPDIR
   
    tar czf skegn-linux-$VERSION-$TIMESTAMP.tar.gz $TMPDIR
    return 0
}

function build_for_mac()
{
    # provison
    make -f Makefile-mac clean all VERSION=$VERSION TIMESTAMP=$TIMESTAMP ; try_catch $?;
    
    #ssl
#    make -f Makefile-mac clean all VERSION=$VERSION TIMESTAMP=$TIMESTAMP USE_SSL=1; try_catch $?;
    
	# package
    TMPDIR=skegn-mac-$VERSION-$TIMESTAMP
    rm -rf $TMPDIR; mkdir $TMPDIR
    #mv skegn-*-$TIMESTAMP.sh $TMPDIR
    mv libskegn-*-$TIMESTAMP.so $TMPDIR
    mv libskegn-*-$TIMESTAMP.a $TMPDIR
    cp include/skegn.h include/SkEgn.java $TMPDIR
 
    tar czf skegn-mac-$VERSION-$TIMESTAMP.tar.gz $TMPDIR
    return 0
}

function build_for_mingw()
{
    #cloud
#    make -f Makefile-mingw clean all MINGWABI=i686 VERSION=$VERSION TIMESTAMP=$TIMESTAMP; try_catch $?;
#    make -f Makefile-mingw clean all MINGWABI=x86_64 VERSION=$VERSION TIMESTAMP=$TIMESTAMP; try_catch $?;
    
    #cloud ssl
#    make -f Makefile-mingw clean all MINGWABI=i686 VERSION=$VERSION TIMESTAMP=$TIMESTAMP USE_SSL=1 ; try_catch $?;
#    make -f Makefile-mingw clean all MINGWABI=x86_64 VERSION=$VERSION TIMESTAMP=$TIMESTAMP USE_SSL=1 ; try_catch $?;
    
    #native
    make -f Makefile-mingw clean all MINGWABI=i686 VERSION=$VERSION TIMESTAMP=$TIMESTAMP USE_NATIVE=1; try_catch $?;
#    make -f Makefile-mingw clean all MINGWABI=x86_64 VERSION=$VERSION TIMESTAMP=$TIMESTAMP USE_NATIVE=1; try_catch $?;
  
    #native ssl 
#    make -f Makefile-mingw clean all MINGWABI=i686 VERSION=$VERSION TIMESTAMP=$TIMESTAMP USE_NATIVE=1 USE_SSL=1; try_catch $?;
#    make -f Makefile-mingw clean all MINGWABI=x86_64 VERSION=$VERSION TIMESTAMP=$TIMESTAMP USE_NATIVE=1 USE_SSL=1; try_catch $?;
  
    # package
    TMPDIR=skegn-mingw-$VERSION-$TIMESTAMP
    rm -rf $TMPDIR; mkdir $TMPDIR
    mv skegn-*.dll skegn-*.lib $TMPDIR
    cp include/skegn.h include/SkEgn.java $TMPDIR

    zip -x *.svn/* -r skegn-mingw-$VERSION-$TIMESTAMP.zip $TMPDIR;
    return 0
}

if [[ $1 != "android" && $1 != "ios" && $1 != "linux" && $1 != "mingw" && $1 != "mac" ]]; then
    echo "invalid target: $1"
    exit
fi

TIMESTAMP=`date +20%y%m%d%H%M%S`
VERSION=`awk -F\" '/SKEGN_VERSION/{print $2}' include/skegn.h`
#VERSION+="-temp-for-f0"
#python shell/extrevert.py skegn-svn-$VERSION-$TIMESTAMP.sh

#get_externals;
build_for_$1; exit $?
