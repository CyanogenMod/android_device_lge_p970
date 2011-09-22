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
mkdir -p ../../../vendor/$VENDOR/$DEVICE/proprietary/etc/omapcam
mkdir -p ../../../vendor/$VENDOR/$DEVICE/proprietary/etc/wifi/softap
mkdir -p ../../../vendor/$VENDOR/$DEVICE/proprietary/etc/cert
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
adb pull /system/lib/libmlplatform.so ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/

# Wifi
adb pull /system/etc/wifi/fw_bcm4329.bin ../../../vendor/$VENDOR/$DEVICE/proprietary/etc/wifi/
adb pull /system/etc/wifi/fw_bcm4329_ap.bin ../../../vendor/$VENDOR/$DEVICE/proprietary/etc/wifi/

# DSP Firmware
adb pull /system/lib/libaffw_2.0.so ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/
adb pull /system/lib/libaf_lg_2.0.so ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/
adb pull /system/bin/fw3a_core ../../../vendor/$VENDOR/$DEVICE/proprietary/bin/
adb pull /system/lib/dsp/720p_h264vdec_sn.dll64P ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/dsp/
adb pull /system/lib/dsp/720p_h264venc_sn.dll64P ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/dsp/
adb pull /system/lib/dsp/720p_mp4vdec_sn.dll64P ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/dsp/
adb pull /system/lib/dsp/720p_mp4venc_sn.dll64P ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/dsp/
adb pull /system/lib/dsp/720p_wmv9vdec_sn.dll64P ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/dsp/
adb pull /system/lib/dsp/baseimage.dof ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/dsp/
adb pull /system/lib/dsp/baseimage.map ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/dsp/
adb pull /system/lib/dsp/chromasuppress.l64p ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/dsp/
adb pull /system/lib/dsp/conversions.dll64P ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/dsp/
adb pull /system/lib/dsp/dctn_dyn.dll64P ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/dsp/
adb pull /system/lib/dsp/ddspbase_tiomap3430.dof64P ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/dsp/
adb pull /system/lib/dsp/dfgm.dll64P ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/dsp/
adb pull /system/lib/dsp/dynbase_tiomap3430.dof64P ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/dsp/
adb pull /system/lib/dsp/eenf_ti.l64P ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/dsp/
adb pull /system/lib/dsp/g711dec_sn.dll64P ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/dsp/
adb pull /system/lib/dsp/g711enc_sn.dll64P ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/dsp/
adb pull /system/lib/dsp/g722dec_sn.dll64P ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/dsp/
adb pull /system/lib/dsp/g722enc_sn.dll64P ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/dsp/
adb pull /system/lib/dsp/g726dec_sn.dll64P ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/dsp/
adb pull /system/lib/dsp/g726enc_sn.dll64P ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/dsp/
adb pull /system/lib/dsp/g729dec_sn.dll64P ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/dsp/
adb pull /system/lib/dsp/g729enc_sn.dll64P ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/dsp/
adb pull /system/lib/dsp/h264vdec_sn.dll64P ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/dsp/
adb pull /system/lib/dsp/h264venc_sn.dll64P ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/dsp/
adb pull /system/lib/dsp/ilbcdec_sn.dll64P ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/dsp/
adb pull /system/lib/dsp/ilbcenc_sn.dll64P ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/dsp/
adb pull /system/lib/dsp/ipp_sn.dll64P ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/dsp/
adb pull /system/lib/dsp/jpegdec_sn.dll64P ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/dsp/
adb pull /system/lib/dsp/jpegenc_sn.dll64P ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/dsp/
adb pull /system/lib/dsp/m4venc_sn.dll64P ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/dsp/
adb pull /system/lib/dsp/monitor_tiomap3430.dof64P ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/dsp/
adb pull /system/lib/dsp/mp3dec_sn.dll64P ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/dsp/
adb pull /system/lib/dsp/mp4vdec_sn.dll64P ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/dsp/
adb pull /system/lib/dsp/mpeg4aacdec_sn.dll64P ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/dsp/
adb pull /system/lib/dsp/mpeg4aacenc_sn.dll64P ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/dsp/
adb pull /system/lib/dsp/nbamrdec_sn.dll64P ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/dsp/
adb pull /system/lib/dsp/nbamrenc_sn.dll64P ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/dsp/
adb pull /system/lib/dsp/postprocessor_dualout.dll64P ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/dsp/
adb pull /system/lib/dsp/qosdyn_3430.dll64P ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/dsp/
adb pull /system/lib/dsp/ringio.dll64P ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/dsp/
adb pull /system/lib/dsp/sparkdec_sn.dll64P ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/dsp/
adb pull /system/lib/dsp/star.l64P ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/dsp/
adb pull /system/lib/dsp/usn.dll64P ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/dsp/
adb pull /system/lib/dsp/vpp_sn.dll64P ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/dsp/
adb pull /system/lib/dsp/wbamrdec_sn.dll64P ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/dsp/
adb pull /system/lib/dsp/wbamrenc_sn.dll64P ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/dsp/
adb pull /system/lib/dsp/wmadec_sn.dll64P ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/dsp/
adb pull /system/lib/dsp/wmv9dec_sn.dll64P ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/dsp/
adb pull /system/lib/dsp/yuvconvert.l64p ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/dsp/

# BT firmware
adb pull /system/etc/firmware/BCM43291A0_003.001.013.0066.xxxx_B-Project.hcd  ../../../vendor/$VENDOR/$DEVICE/proprietary/etc/firmware/

## Camera and related blobs
adb pull /system/lib/libyuvfastconvert.so  ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/
adb pull /system/lib/libicapture.so  ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/
adb pull /system/lib/libicamera.so  ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/
adb pull /system/lib/libcapl.so  ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/
adb pull /system/lib/libcameraalgo.so  ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/
adb pull /system/lib/libcamera.so  ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/
adb pull /system/lib/libarcsoft_camera_func.so  ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/
adb pull /system/lib/libImagePipeline.so  ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/
adb pull /system/etc/omapcam/imx072_dtp.dat  ../../../vendor/$VENDOR/$DEVICE/proprietary/etc/omapcam/
adb pull /system/etc/omapcam/imx072.rev  ../../../vendor/$VENDOR/$DEVICE/proprietary/etc/omapcam/
adb pull /system/etc/omapcam/fw3a.conf  ../../../vendor/$VENDOR/$DEVICE/proprietary/etc/omapcam/

## OMX 720p libraries
adb pull /system/lib/libOMX.TI.mp4.splt.Encoder.so  ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/
adb pull /system/lib/libOMX.TI.720P.Encoder.so  ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/
adb pull /system/lib/libOMX.TI.720P.Decoder.so  ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/
adb pull /system/lib/libOMX.ITTIAM.AAC.encode.so  ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/
adb pull /system/lib/libOMX.ITTIAM.AAC.decode.so  ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/
adb pull /system/lib/libOMX.TI.h264.splt.Encoder.so  ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/

## Audio HAL (temporary until amp problems are fixed)
adb pull /system/lib/libaudio.so  ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/
adb pull /system/lib/hw/alsa.omap3.so  ../../../vendor/$VENDOR/$DEVICE/proprietary/lib/hw/

echo "NOTE: Unless all transfers failed, errors above should be safe to ignore. Proceed with your build"
exit
