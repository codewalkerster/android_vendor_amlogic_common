LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:=     \
    bdt_unisoc.c

LOCAL_C_INCLUDES := $(TOP_DIR)vendor/amlogic/common/wifi_bt/bluetooth/unisoc/tools/btsuite/main/include

LOCAL_MODULE:= bdt_unisoc
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SHARED_LIBRARIES +=
LOCAL_PROPRIETARY_MODULE := true
LOCAL_LICENSE_KINDS := SPDX-license-identifier-Apache-2.0 SPDX-license-identifier-BSD SPDX-license-identifier-LGPL legacy_by_exception_only
LOCAL_LICENSE_CONDITIONS := by_exception_only notice restricted
LOCAL_NOTICE_FILE := $(LOCAL_PATH)/../../../../../LICENSE
include $(BUILD_EXECUTABLE)
