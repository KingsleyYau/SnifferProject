# Copyright (C) 2014 The DrSniffer Project
# drhttpfs Makefile
#
# Created on: 2014年2月14日
# Author: Kingsley Yau
# Email: Kingsleyyau@gmail.com
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

REAL_PATH := $(realpath $(LOCAL_PATH))
commonSources := $(call all-c-files-under, $(REAL_PATH))

# Excutable for target
# ========================================================
include $(CLEAR_VARS)
LOCAL_MODULE := drhttpfs
LOCAL_MODULE_FILENAME := drhttpfs
LOCAL_SRC_FILES := $(commonSources)
LOCAL_STATIC_LIBRARIES := drcommon
LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog
LOCAL_CFLAGS += -g -DPRINT_JNI_LOG -DHAVE_DAEMON
LOCAL_FORCE_STATIC_EXECUTABLE := true
#include $(BUILD_EXECUTABLE)