echo "killing all running"
killall olsrd
killall rsockd
rm rsockd.log
echo "exporting..."
export LD_LIBRARY_PATH=/data/hrpd/lib/
echo "starting olsrd..."
./olsrd -d <DebugLevel> -f ./olsrd.conf > olsrd.log
echo "starting rsockd..."
./rsockd -i <Wifi_or_LTE_Interface_Name> -d > rsockd.log 

