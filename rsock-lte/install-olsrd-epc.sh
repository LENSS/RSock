echo "untaring olsrd-lte-epc ..."
tar -xvf olsrd-lte-epc.tar
echo "building and loading olsrd_jsoninfo.so.1.1  ..."
cd ./olsrd-lte-epc:wq
/lib/jsoninfo
make
make install
echo "building olsrd ..."
cd ../../
make
echo "Done!"


