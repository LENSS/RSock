echo "running iperf for 30 minutes ..."
nohup ./iperf -c 192.168.0.1 -u -t 1800 -b 100M &
echo "done!"


#####################################################################3

# check the bandwidth by sending 30MB file
echo ""
echo "********************************************************************************"
echo "running iperf for 1 hop ..."
iperf -c 10.0.10.2 -u -n 30M -b 100M

echo ""
echo "********************************************************************************"
echo "running iperf for 2 hops ..."
iperf -c 192.168.0.27 -u -n 30M -b 100M

echo "********************************************************************************"

echo "done!"
