#!/sbin/sh

sleep 3

## Reset recovery flags
#boot-recovery
dd if=/dev/zero of=/dev/block/mmcblk0p12 seek=512 count=13 bs=1
#recovery\n--wipe-data
dd if=/dev/zero of=/dev/block/mmcblk0p12 seek=576 count=20 bs=1
