LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

COLLECT_PATCH:=$(shell $(LOCAL_PATH)/collect_patchlist.sh $(LOCAL_PATH) $(OUT_DIR))
$(info collect_patchlist done)

PATCHECK_SRCS := patcheck.cpp
LOCAL_MODULE := check_patch
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(PATCHECK_SRCS)
LOCAL_CFLAGS := $(PATCHECK_CFLAGS)
LOCAL_LICENSE_KINDS := SPDX-license-identifier-Apache-2.0 SPDX-license-identifier-BSD
LOCAL_LICENSE_CONDITIONS := notice

include $(BUILD_EXECUTABLE)
