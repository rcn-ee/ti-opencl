#!/bin/sh
read vendor_value < $1/vendor
read device_value < $1/device
if [ "$vendor_value" = "0x104c" -a "$device_value" = "0xb005" ]
then
read switch_vendor_value < $1/../vendor
read switch_device_value < $1/../device
/usr/bin/find $1 -maxdepth 1 -name "resource*" -exec /bin/chmod ugo+rw {} + 

/bin/date >> /var/log/c6678_udev.log 
/bin/echo $1 : vendor:$vendor_value device:$device_value switch_vendor:$switch_vendor_value switch_device:$switch_device_value >>/var/log/c6678_udev.log
fi
