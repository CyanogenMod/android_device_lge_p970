#USE_CAMERA_STUB := true

# inherit from the proprietary version
-include vendor/lge/p970/BoardConfigVendor.mk

TARGET_BOARD_PLATFORM := omap3

# Board configuration
#
TARGET_ARCH := arm
TARGET_NO_BOOTLOADER := true
TARGET_CPU_ABI := armeabi-v7a
TARGET_CPU_ABI2 := armeabi
TARGET_ARCH_VARIANT := armv7-a-neon
ARCH_ARM_HAVE_TLS_REGISTER := true
OMAP_ENHANCEMENT := true

TARGET_BOOTLOADER_BOARD_NAME := p970

BOARD_KERNEL_CMDLINE :=
BOARD_KERNEL_BASE := 0x80000000
BOARD_PAGE_SIZE := 0x00000800

ifdef OMAP_ENHANCEMENT
COMMON_GLOBAL_CFLAGS += -DOMAP_ENHANCEMENT -DTARGET_OMAP3 -DOMAP_ENHANCEMENT_CPCAM -DOMAP_ENHANCEMENT_VTC
endif

# Try to build the kernel
TARGET_KERNEL_CONFIG := cyanogenmod_p970_defconfig
# Keep this as a fallback
TARGET_PREBUILT_KERNEL := device/lge/p970/kernel

BOARD_NEEDS_CUTILS_LOG := true

BOARD_HAS_NO_SELECT_BUTTON := true
# Use this flag if the board has a ext4 partition larger than 2gb
#BOARD_HAS_LARGE_FILESYSTEM := true

BOARD_HAVE_BLUETOOTH := true
BOARD_HAVE_BLUETOOTH_BCM := true
BOARD_HAVE_FM_RADIO := true

# BOARD_USES_TI_CAMERA_HAL := true
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
#BOARD_CAMERA_LIBRARIES := libcamera
endif

BOARD_MOBILEDATA_INTERFACE_NAME := "vsnet0"

BOARD_WLAN_DEVICE := bcm4329
WIFI_DRIVER_FW_PATH_STA         := "/system/etc/firmware/fw_bcm4329.bin"
WIFI_DRIVER_FW_PATH_AP          := "/system/etc/firmware/fw_bcm4329_ap.bin"
WIFI_DRIVER_MODULE_NAME         := "wireless"
WIFI_DRIVER_MODULE_PATH         := "/system/lib/modules/wireless.ko"
WIFI_DRIVER_MODULE_ARG          := "firmware_path=/system/etc/firmware/fw_bcm4329.bin nvram_path=/system/etc/wifi/nvram.txt config_path=/data/misc/wifi/config"
WPA_SUPPLICANT_VERSION          := VER_0_6_X
#BOARD_HOSTAPD_PRIVATE_LIB := lib_driver_cmd_wext
#BOARD_WPA_SUPPLICANT_PRIVATE_LIB := lib_driver_cmd_wext
WIFI_DRIVER_HAS_LGE_SOFTAP      := true
BOARD_WPA_SUPPLICANT_DRIVER := WEXT
BOARD_WEXT_NO_COMBO_SCAN       := true


BOARD_EGL_CFG := device/lge/p970/configs/egl.cfg

TARGET_USERIMAGES_USE_EXT4 := true
BOARD_SYSTEMIMAGE_PARTITION_SIZE := 665681920
BOARD_USERDATAIMAGE_PARTITION_SIZE := 1170259968
BOARD_FLASH_BLOCK_SIZE := 131072

TARGET_SPECIFIC_HEADER_PATH := device/lge/p970/include

USE_OPENGL_RENDERER := true
#BOARD_TOUCH_RECOVERY := true
BOARD_CUSTOM_GRAPHICS := ../../../device/lge/p970/recovery/graphics.c
#TARGET_RECOVERY_PIXEL_FORMAT := "RGBX_8888"

BOARD_HAS_VIBRATOR_IMPLEMENTATION := ../../device/lge/p970/vibrator.c
COMMON_GLOBAL_CFLAGS += -DICS_AUDIO_BLOB -DICS_CAMERA_BLOB -DOMAP_ICS_CAMERA -DUSE_FENCE_SYNC

BOARD_SYSFS_LIGHT_SENSOR := "/sys/devices/platform/omap/omap_i2c.2/i2c-2/2-0060/leds/lcd-backlight/als"

COMMON_GLOBAL_CFLAGS += -DBOARD_CHARGING_CMDLINE_NAME='"rs"' -DBOARD_CHARGING_CMDLINE_VALUE='"c"'
## Radio fixes
BOARD_RIL_CLASS := ../../../device/lge/p970/ril/

