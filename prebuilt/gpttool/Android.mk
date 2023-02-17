LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := makegpt
LOCAL_SRC_FILES := makegpt
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_TAGS := optional

LOCAL_IS_HOST_MODULE := true

LOCAL_LICENSE_KINDS := SPDX-license-identifier-Apache-2.0 SPDX-license-identifier-BSD SPDX-license-identifier-LGPL legacy_by_exception_only legacy_proprietary
LOCAL_LICENSE_CONDITIONS := by_exception_only notice restricted proprietary by_exception_only
include $(BUILD_PREBUILT)
