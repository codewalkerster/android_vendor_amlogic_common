LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := fm_main.c
LOCAL_SHARED_LIBRARIES := libc libcutils
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_MODULE = fmapp
LOCAL_LICENSE_KINDS := SPDX-license-identifier-Apache-2.0 SPDX-license-identifier-BSD SPDX-license-identifier-GPL-2.0 legacy_proprietary
LOCAL_LICENSE_CONDITIONS := notice restricted proprietary by_exception_only
LOCAL_MODULE_TAGS := optional
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_EXECUTABLE)
