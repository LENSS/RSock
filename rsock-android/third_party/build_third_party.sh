#!/bin/bash

export PROJECT_DIR=`pwd`/../
export STAGING_DIR=$PROJECT_DIR/staging_dir
export ANDROID_NDK=/home/ala/Android/Sdk/ndk-bundle
export ANDROID_STANDALONG_TOOLCHAIN=/home/ala/android-toolchain


mkdir -p $STAGING_DIR

# # build protobuf
tar xzf protobuf-all-3.5.1.tar.gz
cp cross-compile.sh ./protobuf-3.5.1/
cd protobuf-3.5.1
bash cross-compile.sh


# # build lemon
cd ..
tar xzf lemon-1.3.1.tar.gz
cd lemon-1.3.1
mkdir build
cd build

cmake .. \
	-DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK/build/cmake/android.toolchain.cmake" \
    -DANDROID_NDK="$ANDROID_NDK" \
    -DANDROID_TOOLCHAIN=clang \
    -DANDROID_ABI=armeabi-v7a \
    -DANDROID_NATIVE_API_LEVEL=21 \
    -DANDROID_STL=c++_shared \
    -DANDROID_LINKER_FLAGS="-landroid -llog" \
    -DANDROID_CPP_FEATURES="rtti exceptions" \
    -DBUILD_SHARED_LIBS=true \
    -DCMAKE_INSTALL_PREFIX=$STAGING_DIR \
    && make && make install
cd ../../

# # build olsr
echo "Before Instaling OLSRD"
tar xzf olsrd-0.9.6.2.tar.gz
cp Makefile.android* olsrd-0.9.6.2/make/
cd olsrd-0.9.6.2
make OS=android prefix=$STAGING_DIR build_all
make OS=android prefix=$STAGING_DIR install_all
echo "After Instaling OLSRD"
