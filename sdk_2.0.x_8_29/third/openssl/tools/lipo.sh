xcrun -sdk iphoneos lipo -output libssl.a -create -arch armv7 openssl-armv7/lib/libssl.a  \
						-arch armv7s openssl-armv7s/lib/libssl.a  \
						-arch arm64 openssl-arm64/lib/libssl.a  \
						-arch i386 openssl-i386/lib/libssl.a  \
						-arch x86_64 openssl-x86_64/lib/libssl.a  
xcrun -sdk iphoneos lipo -output libcrypto.a -create -arch armv7 openssl-armv7/lib/libcrypto.a \
							 -arch arm64 openssl-arm64/lib/libcrypto.a \
							-arch armv7s openssl-armv7s/lib/libcrypto.a  \
							-arch i386 openssl-i386/lib/libcrypto.a  \
							-arch x86_64 openssl-x86_64/lib/libcrypto.a  
