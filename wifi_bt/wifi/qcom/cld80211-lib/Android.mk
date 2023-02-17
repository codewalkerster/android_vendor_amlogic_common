LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libcld80211-aml
LOCAL_LICENSE_KINDS := SPDX-license-identifier-BSD legacy_proprietary
LOCAL_LICENSE_CONDITIONS := notice proprietary by_exception_only
LOCAL_CLANG := true
LOCAL_MODULE_TAGS := optional
LOCAL_C_INCLUDES += $(LOCAL_PATH) \
	external/libnl/include
LOCAL_SHARED_LIBRARIES := libcutils libnl liblog
LOCAL_SRC_FILES := cld80211_lib.c
LOCAL_CFLAGS += -Wall -Werror -Wno-unused-parameter
#LOCAL_COPY_HEADERS_TO := cld80211-lib
#LOCAL_COPY_HEADERS := cld80211_lib.h
LOCAL_VENDOR_MODULE := true
LOCAL_LICENSE_KINDS := SPDX-license-identifier-BSD legacy_proprietary
LOCAL_LICENSE_CONDITIONS := notice proprietary by_exception_only
include $(BUILD_SHARED_LIBRARY)
