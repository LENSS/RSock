#!/bin/bash
# -------------------------------------------------
#
# Created by Chen
# Copy this file to build/
# Run ./deploy.sh to deploy compiled binaries to
# two other machines. Here ubuntu1 and ubuntu2
# should already be set in /etc/hosts file and
# public/private keys should already be set.
#
# --------------------------------------------------

echo "packing binary files"
tar czf build.tar.gz *

# echo "clearning binary files in ubuntu1"
# ssh chen@ubuntu1 "rm -rf hrp/build && mkdir -p /home/chen/hrp/build"

echo "transmitting binary files to ubuntu1"
scp build.tar.gz chen@ubuntu1:/home/chen/Documents/AcademicCVS/git/hrp/build

echo "unpacking binary in ubuntu1"
ssh chen@ubuntu1 "cd /home/chen/Documents/AcademicCVS/git/hrp/build && tar xzf build.tar.gz"

# echo "clearning binary files in ubuntu2"
# ssh chen@ubuntu2 "rm -rf hrp/build && mkdir -p /home/chen/hrp/build"

echo "transmitting binary files to ubuntu2"
scp build.tar.gz chen@ubuntu2:/home/chen/Documents/AcademicCVS/git/hrp/build

echo "unpacking binary in ubuntu2"
ssh chen@ubuntu2 "cd /home/chen/Documents/AcademicCVS/git/hrp/build && tar xzf build.tar.gz"