echo "killing all running"
killall olsrd
killall rsockd
rm rsockd.log
echo "exporting..."
export LD_LIBRARY_PATH=/data/hrpd/lib/
echo "starting olsrd..."
./olsrd -d 0 -f olsrd.conf
echo "starting rsockd..."
./rsockd -i rmnet_data0 -d > rsockd.log
