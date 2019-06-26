echo "killing all running"
killall -s SIGKILL olsrd
killall -s SIGKILL rsockd
killall -s SIGKILL test_client
sleep 0.5
rm rsockd.log
echo "starting rsockd..."
./rsockd -1 pgwtun -2 eno1 -d -g > rsockd.log

