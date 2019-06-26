
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
$ export LD_LIBRARY_PATH=/data/hrpd/lib/


Make sure that olsrd running in same shell
$ ./olsrd -d 0 -f olsrd.conf 

use the olsrd.conf provided


Start the daemon using this command on two phones on their interfaces:
$ ./rsockd -i p2p-p2p0-0 -d > testd.log 

Start a sender test_client on one device using this command:
$ ./test_client -s -a f1 -d 192.168.0.132 -z 1024 -n 10 -i 0 -l 300 

And start a receiver test_client on the other device using this command:
$ ./test_client -r -a f1 > tclient.log     





