# Ala Altaweel May 25, 2018
# This script to push cross-compiled rsock into phones with below hash strings
#!/bin/bash

#ADB_HASH=("PM1LHMA7C0600967" "PM1LHMA7C0102954" "PM1LHMA7C0605571" "PM1LHMA7C1300443")
#ADB_HASH=("PM1LHMA7C0102954")
#ADB_HASH=("PM1LHMA7C0605571" "PM1LHMA7C1300443")
#ADB_HASH=("PM1LHMA7C0605571" "PM1LHMA7C0600967")
#ADB_HASH=("PM1LHMA7C1301880" )


# Ala's Phones
ADB_HASH=("PM1LHMA7C1301084" "PM1LHMA7C1300443" "PM1LHMA7C1301880")

# Mohammed's Phones
#ADB_HASH=("PM1LHMA7C0605571" "PM1LHMA7C1300284" "PM1LHMA7C0600967")


#ADB_HASH=("PM1LHMA7C1301084" "PM1LHMA7C1300443" "PM1LHMA7C1301880" "PM1LHMA7C0600967" "PM1LHMA7C1300284")

# comment the below lines when run on deployment environment
rm -r release/*
cp -r staging_dir/bin release/
cp -r staging_dir/etc release/
cp -r staging_dir/lib release/


cd release
tar cvf rsock1.tar bin etc
tar cvf rsock2.tar lib
tar -xvf rsock1.tar
tar -xvf rsock2.tar

tar cvf rsock.tar bin etc lib

for hash in "${ADB_HASH[@]}"
do
        echo "pushing rsock.tar to $hash"
        adb -s $hash push rsock.tar /sdcard/hrpd/
        adb -s $hash shell "
        su root -c 'cp -r /sdcard/hrpd/rsock.tar /data/hrpd/'
        echo 'unzipping rsock.tar'
        su root -c 'cd /data/hrpd/ && tar xvf rsock.tar && chmod 777 bin/olsrd && chmod 777 bin/rsockd && chmod 777 bin/test_client'
        "

        echo "pushing flush_iptables.sh to $hash"
        adb -s $hash push ../flush_iptables.sh /sdcard/hrpd/
        adb -s $hash shell "
        su root -c 'cp /sdcard/hrpd/flush_iptables.sh /data/hrpd/bin'
        "
        echo "pushing start-up-phone.sh to $hash"
        adb -s $hash push ../start-up-phone.sh /sdcard/hrpd/
        adb -s $hash shell "
        su root -c 'cp /sdcard/hrpd/start-up-phone.sh /data/hrpd/bin'
        "
	echo "change permission of start-up-phone.sh to 777 $hash"
	adb -s $hash shell "
        su root -c 'chmod 777 /data/hrpd/bin/start-up-phone.sh'
        "


	echo "pushing killall.sh to $hash"
        adb -s $hash push ../killall.sh /sdcard/hrpd/
        adb -s $hash shell "
        su root -c 'cp /sdcard/hrpd/killall.sh /data/hrpd/bin'
        "
	echo "change permission of killall.sh to 777 $hash"
	adb -s $hash shell "
        su root -c 'chmod 777 /data/hrpd/bin/killall.sh'
        "
	
        echo "pushing olsrd.conf to $hash"
        adb -s $hash push ../olsrd.conf /sdcard/hrpd/
        adb -s $hash shell "
        su root -c 'cp /sdcard/hrpd/olsrd.conf /data/hrpd/bin'
        "



done

cd ..
