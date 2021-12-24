LOCAL_PATH := $(my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := mfgbridge
LOCAL_LICENSE_KINDS := SPDX-license-identifier-Apache-2.0 SPDX-license-identifier-BSD SPDX-license-identifier-LGPL legacy_by_exception_only
LOCAL_LICENSE_CONDITIONS := by_exception_only notice restricted
LOCAL_SRC_FILES := mfgbridge.c mfgdebug.c ../drvwrapper/drv_wrapper.c
#LOCAL_MODULE_TAGS := eng development
LOCAL_CFLAGS += -Wno-Wreturn-type -Wno-error -DNONPLUG_SUPPORT -DMFG_UPDATE -DRAWUR_BT_STACK
LOCAL_C_INCLUDES := $(INCLUDES)
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_EXECUTABLE)
