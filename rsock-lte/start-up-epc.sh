echo "killing all running"
killall olsrd
killall rsockd
rm rsockd.log
echo "starting olsrd..."
./olsrd-lte-epc/olsrd -f ./olsrd_epc.conf -d 0
echo "starting rsockd..."
./rsock-master/build/src/rsockd -i pgwtun -d > rsockd.log

