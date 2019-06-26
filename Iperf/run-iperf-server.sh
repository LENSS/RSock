echo "run iperf server ..."
iperf -s -u -t 1800 -i 1
echo "Done!"


#####################################################################

# use this without -i 1 and the server report will be generated at the end (i.e., when receiving the file)
echo "run iperf server ..."
iperf -u -s -t 1800
echo "Done!"
