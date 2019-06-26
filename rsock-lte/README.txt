#By Ala ALtaweel 
July 13, 2018

To run olsrd in the epc:
build and compile olsrd-lte-epc on the epc unix machine:

1. copy olsrd-lte-epc.tar to EPC server using scp

2. untar the folder
	tar -xvf olsrd-lte-epc.tar 
3. make olsrd for EPC

# this is to install and load jsoninfo
cd ./olsrd-lte-EPC/lib/jsoninfo
make
make install

# this is to make olsrd
cd ../../
make


You can use install-olsrd-epc.sh script for make olsrd after you copying it to epc

Copy olsrd_epc.conf to epc
Copy to phone

Run olsrd on EPC:
./start-up-epc.sh

To run olsrd in phones:
./start-up-phone.sh

Notes on Ip4Broadcast on config files:
Ip4Broadcast in epc's config file should be the IP address of the epc in the format xx.xx.xx.1 Here we assume that the netmask is /24 (notice the mask of the IP address (24). That means the clients IP addresses should be .2 .3 ..... .254)
Ip4Broadcast in phones' config file should be the IP address of the epc in the format xx.xx.xx.1

