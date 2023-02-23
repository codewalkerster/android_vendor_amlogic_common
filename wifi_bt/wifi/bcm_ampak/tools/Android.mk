LOCAL_PATH:=$(call my-dir)


include $(CLEAR_VARS)
LOCAL_MODULE := wl
LOCAL_LICENSE_KINDS := SPDX-license-identifier-Apache-2.0 SPDX-license-identifier-BSD SPDX-license-identifier-FTL SPDX-license-identifier-GPL SPDX-license-identifier-ISC SPDX-license-identifier-LGPL SPDX-license-identifier-LGPL-2.1 SPDX-license-identifier-LGPL-3.0 SPDX-license-identifier-MIT SPDX-license-identifier-OpenSSL SPDX-license-identifier-Zlib legacy_by_exception_only legacy_notice legacy_unencumbered legacy_proprietary
LOCAL_LICENSE_CONDITIONS := by_exception_only notice restricted unencumbered proprietary by_exception_only
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC

ifeq ($(shell test $(PLATFORM_SDK_VERSION) -ge 26 && echo OK),OK)
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_PATH  := $(TARGET_OUT_VENDOR)/bin
else
LOCAL_MODULE_PATH  := $(TARGET_OUT)/bin
endif

#LOCAL_SRC_FILES := $(LOCAL_MODULE)
#include $(BUILD_PREBUILT)

#include $(CLEAR_VARS)
#LOCAL_MODULE := dhd
#LOCAL_MODULE_TAGS := optional
#LOCAL_MODULE_CLASS := ETC

#ifeq ($(shell test $(PLATFORM_SDK_VERSION) -ge 26 && echo OK),OK)
#LOCAL_PROPRIETARY_MODULE := true
#LOCAL_MODULE_PATH  := $(TARGET_OUT_VENDOR)/bin
#else
#LOCAL_MODULE_PATH  := $(TARGET_OUT)/bin
#endif

LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

ifeq ($(BCM_USB_WIFI),true)
    include $(LOCAL_PATH)/bcmdl/bcmdl/Android.mk $(LOCAL_PATH)/bcmdl/libusb2/Android.mk $(LOCAL_PATH)/bcmdl/libusb-compat2/Android.mk
endif

ifeq ($(MULTI_WIFI_SUPPORT),true)
    include $(LOCAL_PATH)/bcmdl/bcmdl/Android.mk $(LOCAL_PATH)/bcmdl/libusb2/Android.mk $(LOCAL_PATH)/bcmdl/libusb-compat2/Android.mk
endif
