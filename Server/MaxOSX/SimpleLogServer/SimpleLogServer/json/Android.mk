# Copyright (C) 2014 The DrSniffer Project
# Json Module Makefile
#
# Created on: 2014年2月14日
# Author: Kingsley Yau
# Email: Kingsleyyau@gmail.com
#

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := drjson
LOCAL_MODULE_FILENAME := libdrjson
REAL_PATH := $(realpath $(LOCAL_PATH))
LOCAL_SRC_FILES := $(call all-cpp-files-under, $(REAL_PATH))
#include $(BUILD_SHARED_LIBRARY) 
include $(BUILD_STATIC_LIBRARY) 
