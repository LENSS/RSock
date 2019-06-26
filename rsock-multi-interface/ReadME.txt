
## By Ala Altaweel

## Library of the Hybrid Routing Protocol (HRP) 

## Environment requirements

## Compile in Linux/ubuntu 
1. g++-4.8.3

2. protobuf, v3.3.2 (https://github.com/google/protobuf                  https://github.com/google/protobuf/blob/master/src/README.md?utf8=%E2%9C%93)

3. asio, v1.10.8  (https://launchpad.net/ubuntu/+archive/primary/+files/libasio-dev_1.10.8-1_all.deb) - install using dpkg -i

4. googletest (https://www.eriksmistad.no/getting-started-with-google-test-on-ubuntu/)

5. cmake, v2.6 (https://cmake.org/download/          https://askubuntu.com/questions/355565/how-do-i-install-the-latest-version-of-cmake-from-the-command-line)

6. [lemon, v1.3.1](http://lemon.cs.elte.hu/trac/lemon) (graph library)

7. [rapidjson, v1.1.0](http://rapidjson.org/) (json library)

8. [spdlog, v0.14.0](https://github.com/gabime/spdlog) (logging library)

9. [bloom](https://github.com/ArashPartow/bloom), (bloom filter library)


## By Ala Altaweel April 04, 2018
## IMPORTANT FOR FAKE VIRTUAL BOX MACHINE
## EXPORT THIS MACHINE AND DISTRIBUTE IT ON ALL OTHER MALCIOUS NODES, 
## File->export
## Import on other machines using File->improt
## username: ala
## password: csce
The virutal box for the fake nod is created on my desktop
	1.  $su	
	2.  $virtualbox





Ala Altaweel
May 25, 2018,

To cross compile:
/third_party/./build_third_party.sh

./build.sh

executables generated to
/staging_dir/bin/


to run the libraires that are cross compiled for hrpd
run the below command:

Distribute executables to phones
$ ./dist_to_phones.sh

Export the path for the libraries

export LD_LIBRARY_PATH=/data/hrpd/lib/


Make sure that olsrd running in same shell
$ ./olsrd -d 0 -f olsrd.conf 

use the olsrd.conf provided


Start the daemon using this command on two phones on their interfaces:
$ ./rsockd -i p2p-p2p0-0 -d > testd.log 

Start a sender test_client on one device using this command:
$ ./test_client -s -a f1 -d 192.168.1.141 -z 1024 -n 1 -i 0  

And start a receiver test_client on the other device using this command:
$ ./test_client -r -a f1 > tclient.log     


Path for plugins in unix:
/usr/local/lib/olsrd_jsoninfo.so.1.1
/usr/local/lib/olsrd_nameservice.so.0.4

Path for plugins in Android:
/data/hrpd/lib


/***************************************************************************************************************************/
/***************************************************************************************************************************/
/***************************************************************************************************************************/
/**************************FOR MULTI INTERFACE INTEGRATED WITH GNS**********************************************************/
/***************************************************************************************************************************/
/***************************************************************************************************************************/
/***************************************************************************************************************************/
Start a sender test_client on one device using this command:
./test_client -s -a f1 -d 878D43686D3DC81D2D3586830141E2F297F5AB0A -z 12000 -n 30 -i 0 -l 20000 > s.log &


./test_client -s -a f1 -d 878D43686D3DC81D2D3586830141E2F297F5AB0A -z 1024 -n 100 -i 0 -l 20000


./test_client -s -a f1 -d C39EC832B5FFC877E9160CF18B60F5B256C6476F -z 1024 -n 100 -i 0 -l 20000


And start a receiver test_client on the other device using this command:
./test_client -r -a f1 > r.log &

./test_client -r -a f1


878D43686D3DC81D2D3586830141E2F297F5AB0A	PM1LHMA7C1301084
C39EC832B5FFC877E9160CF18B60F5B256C6476F	PM1LHMA7C1300443
47419AA8025FFD7B15F3E0BF5C30F877269CAA12	PM1LHMA7C1301880

584CB67DC344691F212FFAF1CB20DF4D8F882CCD	EPC


To enable lte and wifi (to see LTE sign on top of the phone) This might cause problem to olsr and phones can not ping EPC sometimes
1. settings -> developer options -> enable "cellular data always active"
2. settings -> more -> turn "Wi-Fi calling" off
USE the below DEFAULT setting of developer options to enable Wi-Fi and LTE
To enable lte and wifi (to NOT see LTE sign on top of the phone)
1. settings -> developer options -> disable "cellular data always active"
2. settings -> more -> turn "Wi-Fi calling" off

The above setting will cause some probelm when wifi and LTE are both enabled (i.e., "LTE signal icon" will have LTE written on it). The problem happens when run olsrd on the phones they can not ping the EPC IP addresses (192.168.1.131 or 192.168.0.1

To solve the above problem:
1. settings -> developer options -> DISAABLE "cellular data always active"
2. settings -> more -> TURN "Wi-Fi calling" on (i.e., it will be Wi-Fi preferred)


To start up automaticall
install init.dInstall app
adb shell
su
mount -o rw,remount /dev/stl12 /system
monkey -p edu.tamu.cse.lenss.gnsservice -v 500




