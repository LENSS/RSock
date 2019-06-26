# Ala Altaweel May 25, 2018
# This script to push cross-compiled rsock into phones with below hash strings
#!/bin/bash

#ADB_HASH=("PM1LHMA7C0600967" "PM1LHMA7C0102954" "PM1LHMA7C0605571" "PM1LHMA7C1300443")
#ADB_HASH=("PM1LHMA7C0102954")
#ADB_HASH=("PM1LHMA7C0605571" "PM1LHMA7C1300443")
ADB_HASH=( "PM1LHMA7C0605571" )
#ADB_HASH=( "PM1LHMA7C0102954")
#ADB_HASH=("PM1LHMA7C1301084" "PM1LHMA7C1300443")

cd release
#tar -xvf rsock1.tar
#tar -xvf rsock2.tar
#rm rsock1.tar
#rm rsock2.tar
tar cvf rsock.tar bin etc lib
cd ..
for hash in "${ADB_HASH[@]}"
do
	cd release
        echo "pushing rsock.tar to $hash"
        adb -s $hash push rsock.tar /sdcard/hrpd/
        adb -s $hash shell "
        su root -c 'cp -r /sdcard/hrpd/rsock.tar /data/hrpd/'
        echo 'unzipping rsock.tar'
        su root -c 'cd /data/hrpd/ && tar xvf rsock.tar && chmod 777 bin/olsrd && chmod 777 bin/rsockd && chmod 777 bin/test_client'
        "
        cd ../
        echo "pushing flush_iptables.sh to $hash"
        adb -s $hash push flush_iptables.sh /sdcard/hrpd/
        adb -s $hash shell "
        su root -c 'cp /sdcard/hrpd/flush_iptables.sh /data/hrpd/bin'
        "
        echo "pushing start-up.sh to $hash"
        adb -s $hash push start-up.sh /sdcard/hrpd/
        adb -s $hash shell "
        su root -c 'cp /sdcard/hrpd/start-up.sh /data/hrpd/bin'
        "
        echo "pushing olsrd.conf to $hash"
        adb -s $hash push olsrd.conf /sdcard/hrpd/
        adb -s $hash shell "
        su root -c 'cp /sdcard/hrpd/olsrd.conf /data/hrpd/bin'
        "



done

cd ..
