echo "killing all running"
killall olsrd
killall rsockd
killall test_client
sleep 0.5
rm rsockd.log
echo "exporting libraries path..."
export LD_LIBRARY_PATH=/data/hrpd/lib/
echo "starting rsockd..."
./rsockd -1 rmnet_data0 -2 wlan0 -d -g > rsockd.log 
#./rsockd -1 rmnet_data0 -2 wlan0 -d > rsockd.log 

