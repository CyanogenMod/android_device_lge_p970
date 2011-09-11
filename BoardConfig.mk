USE_CAMERA_STUB := false

# inherit from the proprietary version
-include vendor/lge/p970/BoardConfigVendor.mk

TARGET_BOARD_PLATFORM := omap3

# Board configuration
#
TARGET_NO_BOOTLOADER := true
TARGET_CPU_ABI := armeabi-v7a
TARGET_CPU_ABI2 := armeabi
TARGET_ARCH_VARIANT := armv7-a-neon
ARCH_ARM_HAVE_TLS_REGISTER := true
TARGET_PROVIDES_INIT_RC := true
OMAP_ENHANCEMENT := true

TARGET_BOOTLOADER_BOARD_NAME := p970

BOARD_KERNEL_CMDLINE := 
BOARD_KERNEL_BASE := 0x80000000
BOARD_PAGE_SIZE := 0x00000800

ifdef OMAP_ENHANCEMENT
COMMON_GLOBAL_CFLAGS += -DOMAP_ENHANCEMENT -DTARGET_OMAP3
endif

TARGET_PREBUILT_KERNEL := device/lge/p970/kernel

BOARD_HAS_NO_SELECT_BUTTON := true
# Use this flag if the board has a ext4 partition larger than 2gb
#BOARD_HAS_LARGE_FILESYSTEM := true

BOARD_HAVE_BLUETOOTH := true
BOARD_HAVE_BLUETOOTH_BCM := true
BOARD_USE_BROADCOM_FM_VOLUME_HACK := true

BOARD_USES_ALSA_AUDIO := true
BUILD_WITH_ALSA_UTILS := true
BOARD_USES_TI_OMAP_MODEM_AUDIO := true

BOARD_USES_TI_CAMERA_HAL := true
HARDWARE_OMX := true
ifdef HARDWARE_OMX
OMX_JPEG := true
OMX_VENDOR := ti
OMX_VENDOR_INCLUDES := \
   hardware/ti/omx/system/src/openmax_il/omx_core/inc \
   hardware/ti/omx/image/src/openmax_il/jpeg_enc/inc
OMX_VENDOR_WRAPPER := TI_OMX_Wrapper
BOARD_OPENCORE_LIBRARIES := libOMX_Core
BOARD_OPENCORE_FLAGS := -DHARDWARE_OMX=1
BOARD_CAMERA_LIBRARIES := libcamera
endif

BOARD_WLAN_DEVICE := bcm4329
WIFI_DRIVER_FW_STA_PATH         := "/system/etc/wifi/fw_bcm4329.bin"
WIFI_DRIVER_FW_AP_PATH          := "/system/etc/wifi/fw_bcm4329_ap.bin"
WIFI_DRIVER_MODULE_NAME         := "wireless"
WIFI_DRIVER_MODULE_PATH         := "/system/lib/modules/wireless.ko"
WIFI_DRIVER_MODULE_ARG          := "firmware_path=/system/etc/wifi/fw_bcm4329.bin nvram_path=/system/etc/wifi/nvram.txt config_path=/data/misc/wifi/config"
#WIFI_DRIVER_MODULE_ARG          := "firmware_path=/system/etc/wifi/fw_bcm4329_p2p.bin nvram_path=/system/etc/wifi/nvram.txt config_path=/data/misc/wifi/config dhd_use_p2p=1"
#WPA_SUPPLICANT_VERSION          := VER_0_6_X
WIFI_DRIVER_HAS_LGE_SOFTAP      := true
BOARD_WPA_SUPPLICANT_DRIVER := WEXT

BOARD_EGL_CFG := device/lge/p970/configs/egl.cfg

TARGET_USERIMAGES_USE_EXT4 := true
BOARD_SYSTEMIMAGE_PARTITION_SIZE := 665681920
BOARD_USERDATAIMAGE_PARTITION_SIZE := 1170259968
BOARD_FLASH_BLOCK_SIZE := 131072

BOARD_OMAP3_WITH_FFC := true
BOARD_HAS_LGE_FFC := true

#BOARD_CAMERA_FORCE_PREVIEWFORMAT := "yuv422i-yuyv"
#BOARD_USES_CAMERAID_PARAM := "video-input"
# Overwrite number of overlay buffers
#COMMON_GLOBAL_CFLAGS += -DOVERLAY_NUM_REQBUFFERS=4
