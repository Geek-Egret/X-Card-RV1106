#! /bin/bash
# update firmware to board
cd fw
adb push aic8800 /oem/usr/ko/
# update password shadow
cd ../
adb push shadow /etc
# update wpa_supplicant.conf
adb push wpa_supplicant.conf /etc
# update chrony.conf
adb push chrony.conf /etc
# add profile.d file
adb push user.sh /etc/profile.d
# add usb_power.sh
adb push usb_power.sh /root
# enter adb shell
adb shell -t "cd /oem/usr/ko && chmod u+x aic8800"
# add aliyun pypi to board
adb shell -t "pip config set global.index-url https://mirrors.aliyun.com/pypi/simple"
adb shell -t "pip config set global.trusted-host mirrors.aliyun.com"
adb shell -t "cd /oem/usr/ko && insmod aic8800_fdrv.ko && cd /etc/init.d && ./S90wlan0 stop && ./S90wlan0 start"
adb shell -t "sync"
