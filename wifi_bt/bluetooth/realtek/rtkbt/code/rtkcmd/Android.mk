LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_SRC_FILES += \
                    rtkcmd.c


LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := rtkcmd
LOCAL_LICENSE_KINDS := SPDX-license-identifier-Apache-2.0
LOCAL_LICENSE_CONDITIONS := notice
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_EXECUTABLE)
