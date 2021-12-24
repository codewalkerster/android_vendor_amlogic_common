ifeq ($(filter sabrina adt3 deadpool atom Beast,$(TARGET_DEVICE)),)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := PlayAutoInstallStub
LOCAL_PRODUCT_MODULE := true
LOCAL_LICENSE_KINDS := SPDX-license-identifier-Apache-2.0 SPDX-license-identifier-BSD SPDX-license-identifier-LGPL legacy_by_exception_only
LOCAL_LICENSE_CONDITIONS := by_exception_only notice restricted
LOCAL_SYSTEM_EXT_MODULE := true
LOCAL_SRC_FILES := PlayAutoInstallStub.apk
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := .apk
LOCAL_BUILT_MODULE_STEM := package.apk
LOCAL_MODULE_TARGET_ARCH := arm
LOCAL_CERTIFICATE := PRESIGNED
LOCAL_DEX_PREOPT := false
include $(BUILD_PREBUILT)

endif
