 编译android:
 执行./_sharead.sh
 然后 编译各种架构
 bash build-openssl1024android.sh android
 进入 openssl-1.0.2s  执行make depend; make ;make install
 然后在上级目录找到生成的 libs/armeabi文件夹
 对于各种架构请分别选择对应参数编译 参数如下:
 android android-armeabi  android64-aarch64 android-x86 android64 android-mips android-mips64

 编译mac:
 ./Configure darwin64-x86_64-cc  
 然后 make ;make install
 
 
 编译IOS：
 bash build-openssl4ios.sh i386
 在上级目录output/ios/openssl-i386 下可以找到生成得库文件
 对应于各个架构得参数分别如下：
 i386 x86_64 armv7 armv7s arm64
 最后将lipo.sh  拷贝到output下执行，合成一个libssl.a 和 libcypto.a