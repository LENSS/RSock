#!/bin/bash

PREFIX=$STAGING_DIR

 
# 1. Use the tools from the Standalone Toolchain
#export ANDROID_STANDALONG_TOOLCHAIN=/Users/chen/.android-toolchains/arm-linux-androideabi-21
export ANDROID_STANDALONG_TOOLCHAIN=/home/ala/android-toolchain

export SYSROOT=$ANDROID_STANDALONG_TOOLCHAIN/sysroot
export CC="$ANDROID_STANDALONG_TOOLCHAIN/bin/arm-linux-androideabi-clang --sysroot $SYSROOT"
export CXX="$ANDROID_STANDALONG_TOOLCHAIN/bin/arm-linux-androideabi-clang++ --sysroot $SYSROOT"
export LD="$ANDROID_STANDALONG_TOOLCHAIN/bin/arm-linux-androideabi-ld"
export AR="$ANDROID_STANDALONG_TOOLCHAIN/bin/arm-linux-androideabi-ar"
export RANLIB="$ANDROID_STANDALONG_TOOLCHAIN/bin/arm-linux-androideabi-ranlib"
export STRIP="$ANDROID_STANDALONG_TOOLCHAIN/bin/arm-linux-androideabi-strip"
#export CXXSTL=$NDK/sources/cxx-stl/gnu-libstdc++/4.6
#export CXXSTL=/usr/local/my-android-toolchain/arm-linux-androideabi/lib
export CXXSTL=/home/ala/android-toolchain/arm-linux-androideabi/lib
##########################################
# Fetch Protobuf 2.5.0 from source.
##########################################

# (
#     cd /tmp
#     curl http://protobuf.googlecode.com/files/protobuf-2.5.0.tar.gz --output /tmp/protobuf-2.5.0.tar.gz
 
#     if [ -d /tmp/protobuf-2.5.0 ]
#     then
#         rm -rf /tmp/protobuf-2.5.0
#     fi
    
#     tar xvf /tmp/protobuf-2.5.0.tar.gz
# )

# cd /tmp/protobuf-2.5.0

# mkdir build-android
 
# 3. Run the configure to target a static library for the ARMv7 ABI
# for using protoc, you need to install protoc to your OS first, or use another protoc by path
./configure \
--prefix=$PREFIX \
--host=arm-linux-androideabi \
--with-sysroot=$SYSROOT \
--enable-cross-compile \
--with-protoc=protoc \
CFLAGS="-march=armv7-a" \
CXXFLAGS="-march=armv7-a" \
LDFLAGS="-llog"
#--disable-shared \
 
# 4. Build
make && make install
 
# 5. Inspect the library architecture specific information
# arm-linux-androideabi-readelf -A build-android/lib/libprotobuf-lite.a

# cp build-android/lib/libprotobuf.a $PREFIX/libprotobuf.a
# cp build-android/lib/libprotobuf-lite.a $PREFIX/libprotobuf-lite.a
