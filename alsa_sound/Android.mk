# hardware/libaudio-alsa/Android.mk
#
# Copyright 2008 Wind River Systems
#

ifeq ($(strip $(BOARD_USES_ALSA_AUDIO)),true)

  LOCAL_PATH := $(call my-dir)

  include $(CLEAR_VARS)

  LOCAL_ARM_MODE := arm
  LOCAL_CFLAGS := -D_POSIX_SOURCE

    LOCAL_C_INCLUDES += external/alsa-lib/include

  LOCAL_SRC_FILES := \
    AudioHardwareALSA.cpp \
    AudioStreamOutALSA.cpp \
    AudioStreamInALSA.cpp \
    ALSAStreamOps.cpp \
    ALSAMixer.cpp \
    ALSAControl.cpp

  LOCAL_MODULE := libaudio
  LOCAL_MODULE_TAGS:= optional

  LOCAL_STATIC_LIBRARIES += libaudiointerface

  LOCAL_SHARED_LIBRARIES := \
    libasound \
    libcutils \
    libutils \
    libmedia \
    libhardware \
    libhardware_legacy \
    libc

ifeq ($(BOARD_HAVE_BLUETOOTH),true)
  LOCAL_SHARED_LIBRARIES += liba2dp
endif

  include $(BUILD_SHARED_LIBRARY)

endif
