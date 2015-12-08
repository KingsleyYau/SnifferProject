# Copyright (C) 2014 The Sniffer Project
# Sinffer Makefile
#
# Created on: 2014年2月14日
# Author: Kingsley Yau
# Email: Kingsleyyau@gmail.com
#
SELF_PATH := $(call my-dir)
include $(call all-makefiles-under, $(SELF_PATH))

LOCAL_PATH := $(SELF_PATH)

include $(CLEAR_VARS)
LOCAL_MODULE := sniffer

LOCAL_MODULE_FILENAME := libsniffer

LOCAL_C_INCLUDES := $(LOCAL_PATH)/task
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../include
LOCAL_C_INCLUDES += $(LIBRARY_PATH)
LOCAL_C_INCLUDES += $(LIBRARY_THIRDY_PARTY_PATH)
LOCAL_C_INCLUDES += $(LIBRARY_THIRDY_PARTY_PATH)/curl/include

LOCAL_CFLAGS := -fpermissive -Wno-write-strings

LOCAL_LDLIBS := -llog -lz

LOCAL_LDFLAGS := -L$(LIBRARY_THIRDY_PARTY_PATH)/openssl/lib/$(TARGET_ARCH)
LOCAL_LDLIBS += -lssl -lcrypto

LOCAL_STATIC_LIBRARIES := common
LOCAL_STATIC_LIBRARIES += json
LOCAL_STATIC_LIBRARIES += httpclient
LOCAL_STATIC_LIBRARIES += crashhandler
LOCAL_STATIC_LIBRARIES += task

LOCAL_CPPFLAGS  := -std=c++11

REAL_PATH := $(realpath $(LOCAL_PATH))
LOCAL_SRC_FILES := $(call all-cpp-files-under, $(REAL_PATH))

LOCAL_FORCE_STATIC_EXECUTABLE := true

include $(BUILD_EXECUTABLE)