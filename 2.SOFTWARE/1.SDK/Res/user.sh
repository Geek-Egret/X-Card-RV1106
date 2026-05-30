#! /bin/sh
device=wlan0
IP_ADDR=$(ifconfig | grep -A1 "$device" | grep 'inet addr:' | cut -d: -f2 | awk '{print $1}')
[ -n "$IP_ADDR" ] && echo "[wlan0] IP: $IP_ADDR"
