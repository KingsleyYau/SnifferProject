# Copyright (C) 2014 The PPOE Project
# DrSinffer Project Main Makefile
#
# Created on: 2014年2月14日
# Author: Kingsley Yau
# Email: Kingsleyyau@gmail.com
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

REAL_PATH := $(realpath $(LOCAL_PATH))
commonSources := $(call all-cpp-files-under, $(REAL_PATH))

# Excutable for target
# ========================================================
include $(CLEAR_VARS)
LOCAL_MODULE := drsniffer
LOCAL_MODULE_FILENAME := drsniffer
LOCAL_SRC_FILES := $(commonSources)
LOCAL_STATIC_LIBRARIES := drcommon drjson
LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog
LOCAL_CFLAGS += -g -DPRINT_JNI_LOG
LOCAL_FORCE_STATIC_EXECUTABLE := true
include $(BUILD_EXECUTABLE)