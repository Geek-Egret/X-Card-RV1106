#! /bin/sh

insmod cfg80211.ko
insmod mac80211.ko
insmod libaes.ko
insmod aes_generic.ko
insmod ccm.ko
insmod gcm.ko

insmod aic8800_bsp.ko
insmod aic8800_fdrv.ko
