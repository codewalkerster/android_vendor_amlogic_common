LOCAL_PATH:= $(call my-dir)

OBJS_c := bcmdl.c usb_linux.c \
          shared/bcmutils.c \
          shared/zlib/adler32.c \
          shared/zlib/inffast.c \
          shared/zlib/inflate.c \
          shared/zlib/infcodes.c \
          shared/zlib/infblock.c \
          shared/zlib/inftrees.c \
          shared/zlib/infutil.c \
          shared/zlib/zutil.c \
          shared/zlib/crc32.c

INCLUDES := $(LOCAL_PATH)/include
INCLUDES += $(LOCAL_PATH)/shared
INCLUDES += $(LOCAL_PATH)/shared/zlib
INCLUDES += $(LOCAL_PATH)/../libusb-compat2/libusb
L_CFLAGS := -DBCMTRXV2 -DTARGETENV_android

include $(CLEAR_VARS)
LOCAL_MODULE := bcmdl
LOCAL_LICENSE_KINDS := SPDX-license-identifier-Apache-2.0 SPDX-license-identifier-BSD SPDX-license-identifier-FTL SPDX-license-identifier-GPL SPDX-license-identifier-ISC SPDX-license-identifier-LGPL-2.1 SPDX-license-identifier-MIT SPDX-license-identifier-OpenSSL SPDX-license-identifier-Zlib legacy_by_exception_only legacy_notice legacy_unencumbered
LOCAL_LICENSE_CONDITIONS := by_exception_only notice restricted unencumbered
LOCAL_MODULE_TAGS := optional

ifeq ($(shell test $(PLATFORM_SDK_VERSION) -ge 26 && echo OK),OK)
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_PATH  := $(TARGET_OUT_VENDOR)/xbin
else
LOCAL_MODULE_PATH  := $(TARGET_OUT_OPTIONAL_EXECUTABLES)
endif

LOCAL_SHARED_LIBRARIES := libcutils liblog
LOCAL_STATIC_LIBRARIES := libusb-compat2 libusb2
LOCAL_CFLAGS = $(L_CFLAGS)
LOCAL_SRC_FILES := $(OBJS_c)
LOCAL_C_INCLUDES := $(INCLUDES)

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE := libbcmdl
LOCAL_LICENSE_KINDS := SPDX-license-identifier-Apache-2.0 SPDX-license-identifier-BSD SPDX-license-identifier-FTL SPDX-license-identifier-GPL SPDX-license-identifier-ISC SPDX-license-identifier-LGPL-2.1 SPDX-license-identifier-MIT SPDX-license-identifier-OpenSSL SPDX-license-identifier-Zlib legacy_by_exception_only legacy_notice legacy_unencumbered
LOCAL_LICENSE_CONDITIONS := by_exception_only notice restricted unencumbered
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := libcutils liblog
LOCAL_STATIC_LIBRARIES := libusb-compat2 libusb2
LOCAL_CFLAGS = $(L_CFLAGS) -DLIB
LOCAL_SRC_FILES := $(OBJS_c)
LOCAL_C_INCLUDES := $(INCLUDES)

ifeq ($(shell test $(PLATFORM_SDK_VERSION) -ge 26 && echo OK),OK)
LOCAL_PROPRIETARY_MODULE := true
endif
include $(BUILD_SHARED_LIBRARY)
