#!/sbin/sh

sleep 3

## Reset recovery flags
cat /dev/zero > /dev/block/mmcblk0p12

echo 0 > /sys/devices/platform/omap/omap_i2c.2/i2c-2/2-001a/blink_enable
