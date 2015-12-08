# Copyright (C) 2014 The Sniffer Project
# Task Module Makefile
#
# Created on: 2014/10/27
# Author: Max.Chiu
# Email: Kingsleyyau@gmail.com
#
LOCAL_PATH := $(call my-dir)
$(info task LOCAL_PATH : $(LOCAL_PATH))
include $(CLEAR_VARS)

LOCAL_MODULE    := task

LOCAL_MODULE_FILENAME := libtask

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../include
LOCAL_C_INCLUDES += $(LIBRARY_PATH)
LOCAL_C_INCLUDES += $(LIBRARY_THIRDY_PARTY_PATH)
LOCAL_C_INCLUDES += $(LIBRARY_THIRDY_PARTY_PATH)/curl/include

LOCAL_STATIC_LIBRARIES := common
LOCAL_STATIC_LIBRARIES += json
LOCAL_STATIC_LIBRARIES += httpclient

LOCAL_CFLAGS := -fpermissive -Wno-write-strings

LOCAL_CPPFLAGS := -std=c++11

REAL_PATH := $(realpath $(LOCAL_PATH))
LOCAL_SRC_FILES := $(call all-cpp-files-under, $(REAL_PATH))

include $(BUILD_STATIC_LIBRARY) 
