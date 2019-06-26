#!/bin/bash
# Modified by Ala Altaweel, May 21, 2018

export PROJECT_DIR=`pwd`
export STAGING_DIR=$PROJECT_DIR/staging_dir
export ANDROID_NDK=/home/ala/Android/Sdk/ndk-bundle

ANDROID_COMPILER="clang"
ANDROID_ABI="armeabi-v7a"
ANDROID_STL="c++_shared"
ANDROID_API_LEVEL="21"

# Commented out by Ala ALtaweel, May 18, 2018
mkdir build
cd build

cmake .. \
    -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK/build/cmake/android.toolchain.cmake" \
    -DCMAKE_FIND_ROOT_PATH="$STAGING_DIR" \
    -DANDROID_NDK=$ANDROID_NDK \
    -DANDROID_TOOLCHAIN=$ANDROID_COMPILER \
    -DANDROID_ABI=$ANDROID_ABI \
    -DANDROID_NATIVE_API_LEVEL=$ANDROID_API_LEVEL \
    -DANDROID_STL=$ANDROID_STL \
    -DANDROID_LINKER_FLAGS="-landroid -llog" \
    -DANDROID_CPP_FEATURES="rtti exceptions" \
    -DCMAKE_INSTALL_PREFIX=$STAGING_DIR \
    -DProtobuf_USE_STATIC_LIBS=ON \
    && make hrpproto \
    && make -j4 VERBOSE=true \
    && make install \
    && cd ..

cp $ANDROID_NDK/sources/cxx-stl/llvm-libc++/libs/$ANDROID_ABI/lib$ANDROID_STL.so $STAGING_DIR/lib/

cp olsrd.conf $STAGING_DIR/etc/olsrd/

