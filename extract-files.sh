#!/bin/sh

# Copyright (C) 2011 The CyanogenMod Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

VENDOR=lge
DEVICE=p970

rm -rf ../../../vendor/$VENDOR/$DEVICE
mkdir -p ../../../vendor/$VENDOR/$DEVICE/proprietary/bin
mkdir -p ../../../vendor/$VENDOR/$DEVICE/proprietary/etc/firmware
mkdir -p ../../../vendor/$VENDOR/$DEVICE/proprietary/etc/wifi
mkdir -p ../../../vendor/$VENDOR/$DEVICE/proprietary/etc/wifi/softap
mkdir -p ../../../vendor/$VENDOR/$DEVICE/proprietary/etc/cert
mkdir -p ../../../vendor/$VENDOR/$DEVICE/proprietary/etc/omapcam/R05_MVEN001_LD2_ND0_IR0_SH0_FL0_SVEN001_DCCID1040
mkdir -p ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/egl
mkdir -p ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/hw

# HAL
adb pull /system/lib/hw/gralloc.omap3.so ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/hw
adb pull /system/lib/hw/lights.omap3.so ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/hw
adb pull /system/lib/hw/sensors.omap3.so ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/hw

## PVRSGX
adb pull /system/lib/egl/libEGL_POWERVR_SGX530_125.so ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/egl
adb pull /system/lib/egl/libGLESv1_CM_POWERVR_SGX530_125.so ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/egl
adb pull /system/lib/egl/libGLESv2_POWERVR_SGX530_125.so ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/egl
adb pull /system/lib/libsrv_um.so ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/
adb pull /system/lib/libsrv_init.so ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/
adb pull /system/lib/libpvr2d.so ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/
adb pull /system/lib/libpvrANDROID_WSEGL.so ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/
adb pull /system/lib/libIMGegl.so ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/
adb pull /system/lib/libglslcompiler.so ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/
adb pull /system/lib/libusc.so ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/
adb pull /system/bin/pvrsrvinit ../../../vendor/$VENDOR/$DEVICE/proprietary/bin/

# Radio

adb pull /system/lib/lge-ril.so ../../../vendor/$VENDOR/$DEVICE/proprietary/lib
adb pull /system/lib/libril.so ../../../vendor/$VENDOR/$DEVICE/proprietary/lib
adb pull /system/lib/libini.so ../../../vendor/$VENDOR/$DEVICE/proprietary/lib


# GPS
adb pull /system/lib/hw/gps.omap3.so ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/hw
adb pull /system/bin/glgps ../../../vendor/$VENDOR/$DEVICE/proprietary/bin/

# Sensors
adb pull /system/lib/libmpl.so ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/
adb pull /system/lib/libmllite.so ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/
adb pull /system/lib/libmlplatform.so ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/libmlplatform.so

# Wifi
adb pull /system/etc/wifi/fw_bcm4329.bin ../../../vendor/$VENDOR/$DEVICE/proprietary/etc/wifi/
adb pull /system/etc/wifi/fw_bcm4329_ap.bin ../../../vendor/$VENDOR/$DEVICE/proprietary/etc/wifi/
adb pull /system/lib/libmllite.so ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/
adb pull /system/lib/libmlplatform.so ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/libmlplatform.so

echo "NOTE: Unless all transfers failed, errors above should be safe to ignore. Proceed with your build"
exit
