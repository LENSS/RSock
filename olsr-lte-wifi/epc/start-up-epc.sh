echo "killing all running"
killall olsrd
killall rsockd
rm rsockd.log
echo "starting olsrd..."
./olsrd -d <DebugLevel> -f ./olsrd_epc.conf > olsrd.log
##################################################################################################################
echo "starting rsockd..."
./rsockd -i <WiFi_Interface_Name> -d > rsockd.log

